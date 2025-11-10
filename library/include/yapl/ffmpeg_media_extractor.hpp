#pragma once

#include "yapl/imedia_extractor.hpp"
#include "yapl/imedia_source.hpp"
#include "yapl/media_info.hpp"
#include "yapl/media_sample.hpp"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/log.h>
#include <libavutil/mem.h>
}

namespace yapl {

struct ffmpeg_media_extractor : public imedia_extractor {
    ffmpeg_media_extractor(std::shared_ptr<imedia_source> mediaSource);
    ~ffmpeg_media_extractor();
    void start() override;
    media_info get_media_info() override;
    std::shared_ptr<media_sample> read_sample() override;

  private:
    std::shared_ptr<imedia_source> m_media_source;
    std::shared_ptr<media_sample> m_media_sample;
    AVPacket m_pkt;
    AVFormatContext *m_fmt_ctx;
    uint8_t *m_avio_buffer;
    AVIOContext *m_avio_ctx;
};

} // namespace yapl
