#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace yapl {

struct media_sample {
    size_t track_id;
    int64_t pts;
    int64_t dts;
    size_t duration;
    std::span<uint8_t> data;
};

struct decoded_media_sample {
    size_t track_id;
    int64_t pts;
    int64_t dts;
    size_t duration;
    std::vector<uint8_t> data;
};

} // namespace yapl
