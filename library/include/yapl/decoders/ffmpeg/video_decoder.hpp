#pragma once

#include "yapl/decoders/idecoder.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"
#include <memory>

namespace yapl::decoders::ffmpeg {

struct video_decoder : public idecoder {
    video_decoder();
    ~video_decoder() override;

    bool decode(std::shared_ptr<track_info> info,
                std::shared_ptr<media_sample> sample,
                std::shared_ptr<decoded_media_sample> decoded_sample) override;
};

} // namespace yapl::decoders::ffmpeg
