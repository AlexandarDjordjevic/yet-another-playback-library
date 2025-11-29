#pragma once

#include "yapl/decoders/i_decoder.hpp"
#include "yapl/track_info.hpp"
#include <memory>
#include <span>

namespace yapl::decoders {

struct i_decoder_factory {
    virtual ~i_decoder_factory() = default;

    virtual std::unique_ptr<i_decoder> create_video_decoder(
        size_t codec_id,
        std::span<uint8_t> extra_data) = 0;

    virtual std::unique_ptr<i_decoder> create_audio_decoder(
        size_t codec_id,
        std::span<uint8_t> extra_data) = 0;
};

} // namespace yapl::decoders
