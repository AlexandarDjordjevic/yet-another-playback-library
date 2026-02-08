#pragma once

#include <cstddef>

namespace yapl {

/// Configuration options for media pipeline
struct pipeline_config {
    /// Video renderer queue size (number of decoded frames)
    /// Higher values = more memory, smoother playback on decode spikes
    size_t video_queue_size = 60;

    /// Audio renderer queue size (number of decoded frames)
    /// Higher values = more memory, smoother playback on decode spikes
    size_t audio_queue_size = 60;

    /// Track queue size (number of encoded samples from extractor)
    /// Higher values = more buffering before playback starts
    size_t track_queue_size = 1024;

    /// Minimum HTTP buffer size in KB before starting playback
    /// Only used for HTTP data sources
    size_t http_buffer_min_kb = 512;
};

} // namespace yapl
