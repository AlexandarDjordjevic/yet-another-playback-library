#pragma once

#include "yapl/i_media_extractor.hpp"
#include "yapl/i_media_source.hpp"
#include <memory>

namespace yapl {

struct i_media_extractor_factory {
    virtual ~i_media_extractor_factory() = default;
    virtual std::unique_ptr<i_media_extractor> create(
        std::shared_ptr<i_media_source> media_source) = 0;
};

} // namespace yapl
