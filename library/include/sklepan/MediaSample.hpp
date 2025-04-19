#pragma once

#include <span>
#include <cstdint>
#include <memory>

namespace sklepan {

struct MediaSample {
    int64_t pts;
    int64_t dts;
    int64_t duration;
    size_t size;
    size_t trackId;
    std::span<uint8_t> data;
};

} // namespace sklepan
