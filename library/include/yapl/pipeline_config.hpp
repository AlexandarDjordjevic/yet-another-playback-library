#pragma once

#include <cstddef>

namespace yapl {

/**
 * @brief Configuration parameters for media pipeline
 *
 * Controls queue sizes and buffering behavior. Larger values increase memory
 * usage but provide smoother playback under variable decode/network performance.
 */
struct pipeline_config {
    /** Video renderer queue capacity (decoded frames). Default: 60 */
    size_t video_queue_size = 60;

    /** Audio renderer queue capacity (decoded frames). Default: 60 */
    size_t audio_queue_size = 60;

    /** Track buffer queue capacity (encoded packets). Default: 1024 */
    size_t track_queue_size = 1024;

    /** Minimum HTTP buffer (KB) before starting playback. Default: 512 */
    size_t http_buffer_min_kb = 512;
};

} // namespace yapl
