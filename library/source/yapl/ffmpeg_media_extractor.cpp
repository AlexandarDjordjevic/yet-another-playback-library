#include "yapl/ffmpeg_media_extractor.hpp"
#include "yapl/debug.hpp"
#include "yapl/imedia_source.hpp"
#include "yapl/media_info.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"

#include <cstdint>
#include <memory>

namespace yapl {

ffmpeg_media_extractor::ffmpeg_media_extractor(
    std::shared_ptr<imedia_source> _media_source)
    : m_media_source{_media_source},
      m_media_sample{std::make_shared<media_sample>()}, m_fmt_ctx{nullptr},
      m_avio_ctx{nullptr} {
    avformat_network_init();

    auto av_read_packet = [](void *opaque, uint8_t *buf, int buf_size) -> int {
        auto media_source = static_cast<imedia_source *>(opaque);
        LOG_DEBUG("ffmpeg_media_extractor - Reading data from media source. "
                  "Buffer size: {}",
                  buf_size);
        auto available = media_source->available();
        LOG_DEBUG("ffmpeg_media_extractor - Available data: {}", available);

        auto read = media_source->read_packet(
            buf_size, {buf, static_cast<size_t>(buf_size)});
        if (read == 0) {
            return AVERROR_EOF;
        }
        LOG_DEBUG("ffmpeg_media_extractor - Read packet: {}", buf_size);
        return buf_size;
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
    LOG_DEBUG("ffmpeg_media_extractor - start");
    if (avformat_open_input(&m_fmt_ctx, nullptr, nullptr, nullptr) < 0) {
        throw std::runtime_error("Could not open input from buffer");
    }
}

ffmpeg_media_extractor::~ffmpeg_media_extractor() {
    LOG_INFO("ffmpeg_media_extractor - Destructor called");
    if (m_avio_ctx) {
        av_free(m_avio_ctx->buffer);
        avio_context_free(&m_avio_ctx);
    }
    if (m_fmt_ctx) {
        avformat_close_input(&m_fmt_ctx);
    }
    avformat_network_deinit();
}

media_info ffmpeg_media_extractor::get_media_info() {
    LOG_INFO("ffmpeg_media_extractor - Getting media info");

    if (avformat_find_stream_info(m_fmt_ctx, nullptr) < 0) {
        throw std::runtime_error("Could not find stream info");
    }

    media_info _media_info;
    _media_info.duration = m_fmt_ctx->duration;
    _media_info.number_of_tracks = m_fmt_ctx->nb_streams;

    for (auto i = 0u; i < m_fmt_ctx->nb_streams; ++i) {
        AVStream *stream = m_fmt_ctx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;

        LOG_INFO("Codec info: id {}, name {}", (int)stream->codecpar->codec_id,
                 avcodec_get_name(stream->codecpar->codec_id));

        track_info _track{};
        _track.track_id = i;
        _track.codec_id = codecpar->codec_id;
        switch (codecpar->codec_type) {
        case AVMEDIA_TYPE_AUDIO: {
            _track.type = track_type::audio;
            _track.properties.audio.sample_rate = codecpar->sample_rate;
            _track.properties.audio.channels = codecpar->ch_layout.nb_channels;
            _track.properties.audio.bit_rate = codecpar->bit_rate;
            _media_info.tracks.push_back(std::make_shared<track_info>(_track));
        } break;
        case AVMEDIA_TYPE_VIDEO: {
            _track.type = track_type::video;
            _track.properties.video.width = codecpar->width;
            _track.properties.video.height = codecpar->height;
            _track.properties.video.frame_rate = av_q2d(stream->avg_frame_rate);
            _track.properties.video.bit_rate = codecpar->bit_rate;
            _media_info.tracks.push_back(std::make_shared<track_info>(_track));
        } break;
        case AVMEDIA_TYPE_SUBTITLE: {
            track_info subtitleTrack;
            subtitleTrack.track_id = i;
            subtitleTrack.type = track_type::subtitle;
            _media_info.tracks.push_back(
                std::make_shared<track_info>(subtitleTrack));
        } break;
        default:
            LOG_WARN("Unknown track type - ID: {}", i);
            break;
        }
    }

    return _media_info;
}

std::shared_ptr<media_sample> ffmpeg_media_extractor::read_sample() {
    if (av_read_frame(m_fmt_ctx, &m_pkt) < 0) {
        LOG_ERROR("ffmpeg_media_extractor - Error reading frame");
        return {};
    }

    if (m_pkt.size <= 0) {
        LOG_ERROR("ffmpeg_media_extractor - Invalid packet size");
        return {};
    }

    if (static_cast<uint32_t>(m_pkt.stream_index) >= m_fmt_ctx->nb_streams) {
        LOG_ERROR("ffmpeg_media_extractor - Invalid stream index");
        return {};
    }

    m_media_sample->track_id = m_pkt.stream_index;
    m_media_sample->pts = m_pkt.pts;
    m_media_sample->dts = m_pkt.dts;
    m_media_sample->duration = m_pkt.duration;
    m_media_sample->data = std::span<uint8_t>(m_pkt.data, m_pkt.size);
    av_packet_unref(&m_pkt);
    return m_media_sample;
}

} // namespace yapl
