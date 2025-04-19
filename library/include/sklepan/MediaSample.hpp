#pragma once

#include "MediaInfo.hpp"

#include <span>

namespace sklepan {

struct MediaSample {
    size_t trackId;
    int64_t pts;
    int64_t dts;  
    std::span<uint8_t> data;  
};

} // namespace sklepan
