#pragma once

#include <cstddef>

namespace yapl {

enum class track_type { unknown, audio, video, subtitle };

struct audio_info {
    size_t sample_rate;
    size_t channels;
    size_t bit_rate;
};

struct video_info {
    size_t width;
    size_t height;
    size_t frame_rate;
    size_t bit_rate;
};

struct track_info {
    size_t track_id;
    track_type type;
    size_t codec_id;
    union {
        audio_info audio;
        video_info video;
    } properties;
};

} // namespace yapl
