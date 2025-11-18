#include "yapl/ffmpeg_media_extractor.hpp"
#include "yapl/debug.hpp"
#include "yapl/imedia_extractor.hpp"
#include "yapl/imedia_source.hpp"
#include "yapl/media_info.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"
#include "yapl/utilities.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <libavcodec/packet.h>
#include <memory>
#include <span>
#include <stdexcept>
#include <vector>

namespace yapl {

ffmpeg_media_extractor::ffmpeg_media_extractor(
    std::shared_ptr<imedia_source> _media_source)
    : m_media_source{_media_source}, m_fmt_ctx{nullptr}, m_avio_ctx{nullptr} {

    avformat_network_init();
    auto av_read_packet = [](void *opaque, uint8_t *buf, int buf_size) -> int {
        auto media_source = static_cast<imedia_source *>(opaque);
        auto read = media_source->read_packet(
            buf_size, {buf, static_cast<size_t>(buf_size)});
        if (read == 0) {
            return AVERROR_EOF;
        }
        return read;
    };

    m_avio_buffer = static_cast<uint8_t *>(av_malloc(4096));
    if (!m_avio_buffer) {
        throw std::runtime_error("Could not allocate AVIO buffer");
    }
    m_avio_ctx =
        avio_alloc_context(m_avio_buffer, 4096, 0, m_media_source.get(),
                           av_read_packet, nullptr, nullptr);
    if (!m_avio_ctx) {
        throw std::runtime_error("Could not alocate AVIOContext");
    }

    m_fmt_ctx = avformat_alloc_context();
    m_fmt_ctx->pb = m_avio_ctx;
    m_fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
}

void ffmpeg_media_extractor::start() {
    if (avformat_open_input(&m_fmt_ctx, nullptr, nullptr, nullptr) < 0) {
        throw std::runtime_error("Could not open input from buffer");
    }
    m_media_info = std::make_shared<media_info>();
    fetch_media_info();
}

ffmpeg_media_extractor::~ffmpeg_media_extractor() {
    if (m_avio_ctx) {
        av_free(m_avio_ctx->buffer);
        avio_context_free(&m_avio_ctx);
    }
    if (m_fmt_ctx) {
        avformat_close_input(&m_fmt_ctx);
    }
    avformat_network_deinit();
}

std::shared_ptr<media_info> ffmpeg_media_extractor::get_media_info() const {
    return m_media_info;
}

void ffmpeg_media_extractor::fetch_media_info() {

    if (avformat_find_stream_info(m_fmt_ctx, nullptr) < 0) {
        throw std::runtime_error("Could not find stream info");
    }

    m_media_info->duration = m_fmt_ctx->duration;
    m_media_info->number_of_tracks = m_fmt_ctx->nb_streams;

    for (auto i = 0u; i < m_fmt_ctx->nb_streams; ++i) {
        AVStream *stream = m_fmt_ctx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;

        track_info _track{};
        _track.track_id = i;
        _track.codec_id = codecpar->codec_id;
        _track.extra_data = std::make_shared<stream_extra_data>(
            std::span<uint8_t>{codecpar->extradata,
                               codecpar->extradata + codecpar->extradata_size});

        switch (codecpar->codec_type) {
        case AVMEDIA_TYPE_AUDIO: {
            _track.type = track_type::audio;
            _track.properties.audio.sample_rate = codecpar->sample_rate;
            _track.properties.audio.channels = codecpar->ch_layout.nb_channels;
            _track.properties.audio.bit_rate = codecpar->bit_rate;
            m_media_info->tracks.push_back(
                std::make_shared<track_info>(_track));
        } break;
        case AVMEDIA_TYPE_VIDEO: {
            _track.type = track_type::video;
            _track.properties.video.width = codecpar->width;
            _track.properties.video.height = codecpar->height;
            _track.properties.video.frame_rate = av_q2d(stream->avg_frame_rate);
            _track.properties.video.bit_rate = codecpar->bit_rate;
            m_media_info->tracks.push_back(
                std::make_shared<track_info>(_track));
        } break;
        case AVMEDIA_TYPE_SUBTITLE: {
            track_info subtitleTrack;
            subtitleTrack.track_id = i;
            subtitleTrack.type = track_type::subtitle;
            m_media_info->tracks.push_back(
                std::make_shared<track_info>(subtitleTrack));
        } break;
        default:
            LOG_WARN("Unknown track type - ID: {}", i);
            break;
        }
    }
}

enum class nal_unit_type {
    unspecified = 0,
    coded_sliece = 1, // non-keyframe
    data_partition_a = 2,
    data_partition_b = 3,
    data_partition_c = 4,
    idr_slice = 5, // keyframe
    sei = 6,
    sps = 7,
    pps = 8,
    aud = 9,
    end_of_sequence = 10,
    end_of_stream = 11,
    filter_data = 12, // padding
};

std::string to_string(nal_unit_type frame) {
    switch (frame) {
    default:
    case nal_unit_type::unspecified:
        return "Unspecified";
    case nal_unit_type::coded_sliece:
        return "Coded Sliece";
    case nal_unit_type::data_partition_a:
        return "Data Partition a";
    case nal_unit_type::data_partition_b:
        return "Data Partition b";
    case nal_unit_type::data_partition_c:
        return "Data Partition c";
    case nal_unit_type::idr_slice:
        return "IDR Slice";
    case nal_unit_type::sei:
        return "Supplemental Enhancement Information (SEI)";
    case nal_unit_type::sps:
        return "Sequence Parameter Set (SPS)";
    case nal_unit_type::pps:
        return "Picture Parameter Set (PPS)";
    case nal_unit_type::aud:
        return "Access Unit Delimiter (AUD)";
    case nal_unit_type::end_of_sequence:
        return "End Of Sequence";
    case nal_unit_type::end_of_stream:
        return "End Of Stream";
    case nal_unit_type::filter_data:
        return "Filter Data";
    }
}

struct avcc_frame {
    avcc_frame(size_t nal_size_len, std::span<uint8_t> raw_data) {
        auto pos = 0;
        for (size_t i = 0; i < nal_size_len; ++i) {
            size = (size << 8) | raw_data[pos++];
        }
        header.value = raw_data[pos];
        auto begin = raw_data.begin() + pos;
        auto end = raw_data.begin() + pos + size;
        data = {begin, end};
    }
    uint32_t size;
    union {
        struct {
            nal_unit_type type : 5;
            uint8_t ref_idc : 2;
            uint8_t header : 1;
        } fields;
        uint8_t value;
    } header;
    std::span<uint8_t> data;
};

read_sample_result ffmpeg_media_extractor::read_sample() {
    read_sample_result output;

    utilities::raii_cleanup cl{[&] { av_packet_unref(&m_pkt); }};

    auto read_frame_result = av_read_frame(m_fmt_ctx, &m_pkt);
    auto stream_id = static_cast<size_t>(m_pkt.stream_index);
    if (read_frame_result < 0) {

        if (AVERROR_EOF) {
            LOG_INFO("ffmpeg_media_extractor - Track {} reached EOS",
                     m_pkt.stream_index);
            return {.stream_id = stream_id,
                    .error = read_sample_error_t::end_of_stream,
                    .sample = {}};
        }

        char buffer[1024]{0};
        av_strerror(read_frame_result, buffer, 1024);
        LOG_CRITICAL("[FFMPEG Media Extractor] Read frame error: {}.", buffer);
        return {.stream_id = stream_id,
                .error = read_sample_error_t::invalid_sample,
                .sample = {}};
    }

    if (m_pkt.size <= 0) {
        LOG_ERROR("ffmpeg_media_extractor - Invalid packet size <= 0");
        return {.stream_id = stream_id,
                .error = read_sample_error_t::invalid_packet_size,
                .sample = {}};
    }

    if (static_cast<uint32_t>(m_pkt.stream_index) >= m_fmt_ctx->nb_streams) {
        LOG_ERROR("ffmpeg_media_extractor - Invalid stream index");
        return {.stream_id = stream_id,
                .error = read_sample_error_t::invalid_stream_index,
                .sample = {}};
    }

    static size_t debug_id{0};

    auto sample = std::make_shared<media_sample>();
    sample->debug_id = debug_id++;
    sample->track_id = m_pkt.stream_index;
    sample->pts = m_pkt.pts;
    sample->dts = m_pkt.dts;
    sample->duration = m_pkt.duration;

    LOG_TRACE("Read sample {}, pts {}, dts {}", sample->debug_id, sample->pts,
              sample->dts);

    packet_to_annexb(
        m_media_info->tracks[sample->track_id]->extra_data->nal_size_length,
        get_media_info(), m_pkt, sample);

    return {.stream_id = stream_id,
            .error = read_sample_error_t::no_errror,
            .sample = sample};
}

ffmpeg_media_extractor::packet_format
ffmpeg_media_extractor::determine_packet_format(size_t nal_size_len,
                                                const AVPacket &pkt) {
    const uint8_t *data = pkt.data;
    const size_t size = pkt.size;

    if (size < 4)
        return packet_format::raw_nal_payload;

    if (nal_size_len < 1 || nal_size_len > 4) {
        return packet_format::raw_nal_payload;
    }

    // annex-b
    if (nal_size_len == 3 || nal_size_len == 4) {
        auto annex_b_header = &"\x00\x00\x00\x01"[nal_size_len - 4];
        if (size >= nal_size_len &&
            memcmp(data, annex_b_header, nal_size_len) == 0) {
            return packet_format::annexb;
        }
    }

    // avcc
    uint32_t declared = 0;
    for (size_t i = 0; i < nal_size_len; i++)
        declared = (declared << 8) | data[i];

    if (declared > 0 && declared <= size - nal_size_len)
        return packet_format::avcc;

    return packet_format::unknown;
}

void ffmpeg_media_extractor::packet_to_annexb(
    size_t nal_size_length, std::shared_ptr<media_info> _media_info,
    AVPacket &pkt, std::shared_ptr<media_sample> &sample) {
    const auto fmt = determine_packet_format(nal_size_length, pkt);
    auto *position = pkt.data;
    const auto *end = pkt.data + pkt.size;

    static const uint8_t sc4[4] = {0, 0, 0, 1};
    switch (fmt) {
    case packet_format::annexb:
        for (auto x : std::span<uint8_t>{pkt.data, pkt.data + pkt.size}) {
            std::cout << (int)x << " ";
        }
        std::cout << std::endl;
        throw std::runtime_error(
            fmt::format("Invalid format {}", static_cast<int>(fmt)));
        sample->data.assign(pkt.data, pkt.data + pkt.size);
        return;

    case packet_format::raw_nal_payload:
        sample->data.insert(sample->data.end(), sc4, sc4 + 4);
        sample->data.insert(sample->data.end(), pkt.data, pkt.data + pkt.size);
        return;

    case packet_format::avcc:
        while (position + nal_size_length <= end) {
            avcc_frame frame{nal_size_length, {position, pkt.data + pkt.size}};

            if (frame.size == 0 ||
                frame.header.fields.type == nal_unit_type::unspecified) {
                LOG_INFO("Nu size {}, type {}", frame.size,
                         to_string(frame.header.fields.type));
            }
            if (frame.header.fields.type == nal_unit_type::end_of_stream) {
                LOG_INFO("ffmpeg extractor - EOS detected!");
            }

            LOG_INFO("Frame type {}", to_string(frame.header.fields.type));

            if (frame.header.fields.type == nal_unit_type::idr_slice ||
                frame.header.fields.type == nal_unit_type::coded_sliece) {
                auto &sps_data =
                    _media_info->tracks[pkt.stream_index]->extra_data->sps_data;
                auto &pps_data =
                    _media_info->tracks[pkt.stream_index]->extra_data->pps_data;
                sample->data.insert(sample->data.end(), sc4, sc4 + 4);
                sample->data.insert(sample->data.end(), sps_data.begin(),
                                    sps_data.end());
                sample->data.insert(sample->data.end(), sc4, sc4 + 4);
                sample->data.insert(sample->data.end(), pps_data.begin(),
                                    pps_data.end());
            }

            sample->data.insert(sample->data.end(), sc4, sc4 + 4);
            sample->data.insert(sample->data.end(), frame.data.begin(),
                                frame.data.end());

            position += nal_size_length + frame.size;
        }
        return;

    default:
        LOG_ERROR("Unknown packet format!");
        return;
    }
}

} // namespace yapl
