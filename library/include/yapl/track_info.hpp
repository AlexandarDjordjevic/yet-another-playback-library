#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace yapl {

enum class track_type { unknown, audio, video, subtitle };

// Compile-time track type to string conversion
constexpr std::string_view track_type_to_string(track_type type) noexcept {
    switch (type) {
        case track_type::audio:    return "audio";
        case track_type::video:    return "video";
        case track_type::subtitle: return "subtitle";
        default:                   return "unknown";
    }
}

// Constexpr bit extraction helpers for AVC parsing
constexpr uint8_t extract_nal_size_length(uint8_t byte) noexcept {
    return (byte & 0b00000011) + 1;
}

constexpr uint8_t extract_sps_count(uint8_t byte) noexcept {
    return byte & 0b00011111;
}

struct video_extra_data {
    explicit video_extra_data(std::span<uint8_t> data) {
        raw_data.assign(data.begin(), data.end());
        configuration_version = data[0];
        avc_profile_indication = data[1];
        profile_compatibility = data[2];
        avc_level_indication = data[3];
        nal_size_length = extract_nal_size_length(data[4]);
        sps_count = extract_sps_count(data[5]);
        sps_length = static_cast<uint16_t>(data[6]) << 8 | data[7];
        sps_data.assign(&data[8], &data[8 + sps_length]);
        pps_count = data[8 + sps_length];
        pps_length = static_cast<uint16_t>(data[9 + sps_length]) << 8 |
                     data[10 + sps_length];
        pps_data.assign(data.begin() + 11 + sps_length, data.end());
    }

    uint8_t configuration_version;
    uint8_t avc_profile_indication;
    uint8_t profile_compatibility;
    uint8_t avc_level_indication;
    uint8_t nal_size_length; 
    uint8_t sps_count; 
    uint16_t sps_length;
    std::vector<uint8_t> sps_data;
    uint8_t pps_count;
    uint16_t pps_length{0};
    std::vector<uint8_t> pps_data;
    std::vector<uint8_t> raw_data;
};

struct audio_extra_data {
    audio_extra_data(std::span<uint8_t> _data) {
        data.assign(_data.begin(), _data.end());
    }

    std::vector<uint8_t> data;
};

struct audio_track_uniques {
    size_t sample_rate;
    size_t channels;
    size_t bit_rate;
    std::shared_ptr<audio_extra_data> extra_data;
};

struct video_track_uniques {
    static constexpr uint8_t nal_start_code[4] = {0, 0, 0, 1};

    size_t width;
    size_t height;
    double frame_rate;
    size_t bit_rate;
    std::shared_ptr<video_extra_data> extra_data;

    std::vector<uint8_t> get_extra_data() const {
        std::vector<uint8_t> result;
        result.reserve(sizeof(nal_start_code) * 2 +
                       extra_data->sps_data.size() +
                       extra_data->pps_data.size());

        result.insert(result.end(), std::begin(nal_start_code),
                      std::end(nal_start_code));
        result.insert(result.end(), extra_data->sps_data.begin(),
                      extra_data->sps_data.end());
        result.insert(result.end(), std::begin(nal_start_code),
                      std::end(nal_start_code));
        result.insert(result.end(), extra_data->pps_data.begin(),
                      extra_data->pps_data.end());

        return result;
    }
};

using audio_track_specifics_t =
    std::optional<std::shared_ptr<audio_track_uniques>>;
using video_track_specifics_t =
    std::optional<std::shared_ptr<video_track_uniques>>;

struct track_info {
    track_type type{track_type::unknown};
    size_t track_id;
    size_t codec_id;
    video_track_specifics_t video{std::nullopt};
    audio_track_specifics_t audio{std::nullopt};
};

} // namespace yapl
