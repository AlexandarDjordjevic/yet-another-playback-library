#pragma once

#include <cstdint>
#include <vector>

namespace yapl {

struct media_sample {
    size_t debug_id;
    size_t track_id;
    int64_t pts;
    int64_t dts;
    size_t duration;
    std::vector<uint8_t> data;
};

} // namespace yapl
