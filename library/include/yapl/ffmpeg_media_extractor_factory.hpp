#pragma once

#include "yapl/detail/ffmpeg_media_extractor.hpp"
#include "yapl/i_media_extractor_factory.hpp"

namespace yapl {

struct ffmpeg_media_extractor_factory : i_media_extractor_factory {
    std::unique_ptr<i_media_extractor> create(
        std::shared_ptr<i_media_source> media_source) override {
        return std::make_unique<ffmpeg_media_extractor>(std::move(media_source));
    }
};

} // namespace yapl
