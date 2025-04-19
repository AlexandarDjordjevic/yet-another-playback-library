#pragma once

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/mem.h>
    #include <libavutil/log.h>
}

#include "IMediaExtractor.hpp"
#include "IMediaSource.hpp"

namespace sklepan {

struct FFMpegMediaExtractor : public IMediaExtractor {
    FFMpegMediaExtractor(std::shared_ptr<IMediaSource> mediaSource);
    ~FFMpegMediaExtractor();
    void start();
    MediaInfo getMediaInfo() override;
    std::shared_ptr<MediaSample> readSample() override;

private:
    std::shared_ptr<IMediaSource> _mediaSource;
    std::shared_ptr<MediaSample> _mediaSample;
    AVPacket pkt;
    AVFormatContext* fmt_ctx;
    uint8_t * avioBuffer;
    std::shared_ptr<AVIOContext> avio_ctx;
    AVIOContext* avioCtx;
    AVFormatContext* fmtCtx;
};

} // namespace sklepan
