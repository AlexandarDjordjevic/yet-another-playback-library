#pragma once

#include "yapl/imedia_extractor.hpp"
#include "yapl/imedia_source.hpp"
#include "yapl/media_info.hpp"
#include "yapl/media_sample.hpp"
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/log.h>
#include <libavutil/mem.h>
}

namespace yapl {

struct ffmpeg_media_extractor : public imedia_extractor {
  private:
    enum class packet_format { unknown, annexb, avcc, raw_nal_payload };

  public:
    ffmpeg_media_extractor(std::shared_ptr<imedia_source> mediaSource);

    ~ffmpeg_media_extractor();

    void start() override;

    std::shared_ptr<media_info> get_media_info() const override;

    read_sample_result read_sample() override;

  private:
    size_t get_nal_header_len() const;

    void fetch_media_info();

    static packet_format determine_packet_format(size_t nal_size_len,
                                                 const std::span<uint8_t>);

    static void packet_to_annexb(size_t nal_size_length, AVPacket &pkt,
                                 std::shared_ptr<media_sample> &sample);
    // void packet_to_annexb(AVPacket &pkt,
    //                       std::shared_ptr<media_sample> &sample) const;

  private:
    std::shared_ptr<imedia_source> m_media_source;

    std::shared_ptr<media_info> m_media_info;

    AVPacket m_pkt;

    AVFormatContext *m_fmt_ctx;

    uint8_t *m_avio_buffer;

    AVIOContext *m_avio_ctx;
};

} // namespace yapl
