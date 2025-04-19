#pragma once 

#include "MediaInfo.hpp"

#include <memory>
#include <cstddef>

namespace sklepan {

struct Sample {
    MediaInfo mediaInfo;
    size_t sampleSize;
    std::shared_ptr<uint8_t> data;
};

} // namespace sklepan
