#pragma once

#include "yapl/detail/decoders/ffmpeg/audio_decoder.hpp"
#include "yapl/detail/decoders/ffmpeg/video_decoder.hpp"
#include "yapl/decoders/i_decoder_factory.hpp"

namespace yapl::decoders::ffmpeg {

struct ffmpeg_decoder_factory : i_decoder_factory {
    std::unique_ptr<i_decoder> create_video_decoder(
        size_t codec_id,
        std::span<uint8_t> extra_data) override {
        return std::make_unique<video_decoder>(
            static_cast<AVCodecID>(codec_id), extra_data);
    }

    std::unique_ptr<i_decoder> create_audio_decoder(
        size_t codec_id,
        std::span<uint8_t> extra_data) override {
        return std::make_unique<audio_decoder>(
            static_cast<AVCodecID>(codec_id), extra_data);
    }
};

} // namespace yapl::decoders::ffmpeg
