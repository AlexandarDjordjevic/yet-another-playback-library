#pragma once

#include "yapl/media_info.hpp"
#include "yapl/media_sample.hpp"

namespace yapl {

struct i_media_extractor {
    virtual ~i_media_extractor() = default;
    virtual void start() = 0;
    virtual std::shared_ptr<media_info> get_media_info() const = 0;
    virtual read_sample_result read_sample() = 0;
};

} // namespace yapl
