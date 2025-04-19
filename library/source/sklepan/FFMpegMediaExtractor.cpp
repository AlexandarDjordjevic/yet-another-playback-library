#include "sklepan/FFMpegMediaExtractor.hpp"
#include "sklepan/debug.hpp"



#include <cstdint>

namespace sklepan {

FFMpegMediaExtractor::FFMpegMediaExtractor(std::shared_ptr<IMediaSource> mediaSource) 
    : _mediaSource{mediaSource}, _mediaSample{std::make_shared<MediaSample>()}, fmtCtx{nullptr}, avioCtx{nullptr} {
    avformat_network_init();

    auto av_read_packet = [](void* opaque, uint8_t* buf, int buf_size) -> int {
        auto mediaSource = static_cast<IMediaSource*>(opaque);
        LOG_DEBUG("FFMpegMediaExtractor - Reading data from media source. Buffer size: {}", buf_size);
        auto available = mediaSource->available();
        LOG_DEBUG("FFMpegMediaExtractor - Available data: {}", available);
    
        auto read = mediaSource->readPacket(buf_size, {buf, static_cast<size_t>(buf_size)});
        if (read == 0) {
            return AVERROR_EOF;
        }
        LOG_INFO("FFMpegMediaExtractor - Read packet: {}", buf_size);
        return buf_size;
    };

    avioBuffer = static_cast<uint8_t*>(av_malloc(4096));
    if (!avioBuffer) {
        throw std::runtime_error("Could not allocate AVIO buffer");
    }
    avioCtx = avio_alloc_context(avioBuffer, 4096, 0, _mediaSource.get(), av_read_packet, nullptr, nullptr);
    if (!avioCtx) {
        throw std::runtime_error("Could not alocate AVIOContext");
    }

    fmtCtx = avformat_alloc_context();
    fmtCtx->pb = avioCtx;
    fmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;
}

void FFMpegMediaExtractor::start() {
    LOG_INFO("FFMpegMediaExtractor - Starting extraction");
    if (avformat_open_input(&fmtCtx, nullptr, nullptr, nullptr) < 0) {
        throw std::runtime_error("Could not open input from buffer");
    }
}

FFMpegMediaExtractor::~FFMpegMediaExtractor() {
    LOG_INFO("FFMpegMediaExtractor - Destructor called");
    if (avioCtx) {
        av_free(avioCtx->buffer);
        avio_context_free(&avioCtx);
    }
    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
    }
    avformat_network_deinit();
}

MediaInfo FFMpegMediaExtractor::getMediaInfo() {
    
    LOG_INFO("FFMpegMediaExtractor - Getting media info");

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        throw std::runtime_error("Could not find stream info");
    }
    
    MediaInfo mediaInfo;
    mediaInfo.duration = fmtCtx->duration;
    mediaInfo.numbersOfTracks = fmtCtx->nb_streams;

    for (auto i = 0; i < fmtCtx->nb_streams; ++i) {
        AVStream* stream = fmtCtx->streams[i];
        AVCodecParameters* codecpar = stream->codecpar;
        switch (codecpar->codec_type) {
            case AVMEDIA_TYPE_AUDIO: {
                AudioTrackInfo audioTrack;
                audioTrack.type = TrackType::audio;
                audioTrack.trackId = i;
                audioTrack.sampleRate = codecpar->sample_rate;
                audioTrack.channels = codecpar->ch_layout.nb_channels;
                audioTrack.bitRate = codecpar->bit_rate;
                mediaInfo.tracks.push_back(std::make_shared<AudioTrackInfo>(audioTrack));
            } break;
            case AVMEDIA_TYPE_VIDEO: {
                LOG_DEBUG("FFMpegMediaExtractor - Video track found");
                LOG_DEBUG("FFMpegMediaExtractor - Width: {}", codecpar->width);
                LOG_DEBUG("FFMpegMediaExtractor - Height: {}", codecpar->height);
                VideoTrackInfo videoTrack;
                videoTrack.trackId = i;
                videoTrack.type = TrackType::video;
                videoTrack.width = codecpar->width;
                videoTrack.height = codecpar->height;
                videoTrack.frameRate = av_q2d(stream->avg_frame_rate);
                videoTrack.bitRate = codecpar->bit_rate;
                mediaInfo.tracks.push_back(std::make_shared<VideoTrackInfo>(videoTrack));
            } break;
            case AVMEDIA_TYPE_SUBTITLE: {
                TrackInfo subtitleTrack;
                subtitleTrack.trackId = i;
                subtitleTrack.type = TrackType::subtitle;
                mediaInfo.tracks.push_back(std::make_shared<TrackInfo>(subtitleTrack));
            } break;
            default:
                LOG_WARN("Unknown track type - ID: {}", i);
                break;
        }
    }

    return mediaInfo;
}

std::shared_ptr<MediaSample> FFMpegMediaExtractor::readSample() {
    LOG_INFO("FFMpegMediaExtractor - Reading sample");
    if (av_read_frame(fmtCtx, &pkt) < 0) {
        LOG_ERROR("FFMpegMediaExtractor - Error reading frame");
        return {};
    }

    if (pkt.size <= 0) {
        LOG_ERROR("FFMpegMediaExtractor - Invalid packet size");
        return {};
    }

    if (pkt.stream_index >= fmtCtx->nb_streams) {
        LOG_ERROR("FFMpegMediaExtractor - Invalid stream index");
        return {};
    }

    LOG_DEBUG("FFMpegMediaExtractor - Read packet: stream_index: {}, pts: {}, dts: {}, size: {}", pkt.stream_index, pkt.pts, pkt.dts, pkt.size);
    _mediaSample->trackId = pkt.stream_index;
    _mediaSample->pts = pkt.pts;
    _mediaSample->dts = pkt.dts;
    _mediaSample->data = std::span<uint8_t>(pkt.data, pkt.size);
    av_packet_unref(&pkt);
    return _mediaSample;
}


} // namespace sklepan
