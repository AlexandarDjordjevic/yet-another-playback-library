#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace yapl {

enum class track_type { unknown, audio, video, subtitle };

struct video_extra_data {
    video_extra_data(std::span<uint8_t> data) {
        raw_data.assign(data.begin(), data.end());
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
        pps_data.assign(data.begin() + 11 + sps_length, data.end());
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
    std::vector<uint8_t> raw_data;
};

struct audio_extra_data {
    audio_extra_data(std::span<uint8_t> _data) {
        if (_data.size() < 2 || _data.size() > 5) {
            throw std::runtime_error{"Unexpected audio extra data size!"};
        }
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
    size_t width;
    size_t height;
    double frame_rate;
    size_t bit_rate;
    std::shared_ptr<video_extra_data> extra_data;

    inline std::vector<uint8_t> get_extra_data() {
        static auto video_extra_data = [&]() -> std::vector<uint8_t> {
            static const uint8_t sc4[4] = {0, 0, 0, 1};
            std::vector<uint8_t> tmp;
            tmp.assign(sc4, sc4 + 4);
            tmp.insert(tmp.end(), extra_data->sps_data.begin(),
                       extra_data->sps_data.end());
            tmp.insert(tmp.end(), sc4, sc4 + 4);
            tmp.insert(tmp.end(), extra_data->pps_data.begin(),
                       extra_data->pps_data.end());
            return tmp;
        }();
        return video_extra_data;
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
