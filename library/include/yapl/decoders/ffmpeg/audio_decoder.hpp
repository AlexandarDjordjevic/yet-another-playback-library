#pragma once

#include "yapl/decoders/i_decoder.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/codec_par.h>
#ifdef __cplusplus
}
#endif

#include <memory>

namespace yapl::decoders::ffmpeg {

struct audio_decoder : public i_decoder {
    audio_decoder(AVCodecID codec_id, std::span<uint8_t> extra_data);
    ~audio_decoder() override;

    bool decode(std::shared_ptr<track_info> info,
                std::shared_ptr<media_sample> sample,
                std::shared_ptr<media_sample> decoded_sample) override;

  private:
    AVCodecParameters *m_codecpar;
    AVCodecContext *m_codec_ctx;
};

} // namespace yapl::decoders::ffmpeg
