#pragma once

#include <cstdint>
#include <memory>
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

enum class read_sample_error_t {
    no_errror,
    invalid_sample,
    invalid_packet_size,
    invalid_stream_index,
    timeout,
    end_of_stream
};

struct read_sample_result {
    size_t stream_id;
    read_sample_error_t error;
    std::shared_ptr<media_sample> sample;
};

} // namespace yapl
