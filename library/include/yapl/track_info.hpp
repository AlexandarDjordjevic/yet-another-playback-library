#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

namespace yapl {

enum class track_type { unknown, audio, video, subtitle };

struct stream_extra_data {
    stream_extra_data(std::span<uint8_t> data) {
        configuration_version = data[0];
        avc_profile_indication = data[1];
        profile_compatibility = data[2];
        avc_level_indication = data[3];
        nal_size_length = (data[4] & 0b00000011) + 1;
        sps_count = data[5] & 0b00011111;
        sps_length = static_cast<uint16_t>(data[6]) << 8 | data[7];
        sps_data.assign(&data[8], &data[8 + sps_length]);
        pps_count = data[8 + sps_length];
        pps_length = static_cast<uint16_t>(data[9 + sps_length]) << 8 |
                     data[10 + sps_length];
        pps_data.assign(&data[11 + sps_length],
                        &data[11 + sps_length + pps_length]);
    }

    uint8_t configuration_version;
    uint8_t avc_profile_indication;
    uint8_t profile_compatibility;
    uint8_t avc_level_indication;
    uint8_t nal_size_length; // 111111 + nal_length_size_minus1 (3 or 4)
    uint8_t sps_count;       // 111 + numOfSPS
    uint16_t sps_length;
    std::vector<uint8_t> sps_data;
    uint8_t pps_count;
    uint16_t pps_length{0};
    std::vector<uint8_t> pps_data;
};

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
    std::shared_ptr<stream_extra_data> extra_data;
    union {
        audio_info audio;
        video_info video;
    } properties;
};

} // namespace yapl
