#pragma once

#include "yapl/track_info.hpp"

#include <cstddef>
#include <fmt/format.h>
#include <memory>
#include <string>
#include <vector>

namespace yapl {

struct media_info {
    size_t number_of_tracks;
    size_t duration;
    std::vector<std::shared_ptr<track_info>> tracks;
};

static inline std::string to_string(const media_info &media_info) {
    std::string out;
    out += fmt::format("[Media info]: Duration: {}  Number of tracks: {} - ",
                       media_info.duration, media_info.number_of_tracks);

    // Group tracks by type
    std::vector<std::shared_ptr<track_info>> audio_tracks;
    std::vector<std::shared_ptr<track_info>> video_tracks;
    std::vector<std::shared_ptr<track_info>> subtitle_tracks;
    std::vector<std::shared_ptr<track_info>> unknown_tracks;

    for (const auto &track : media_info.tracks) {
        switch (track->type) {
        case track_type::audio:
            audio_tracks.push_back(track);
            break;
        case track_type::video:
            video_tracks.push_back(track);
            break;
        case track_type::subtitle:
            subtitle_tracks.push_back(track);
            break;
        default:
            unknown_tracks.push_back(track);
            break;
        }
    }

    if (!audio_tracks.empty()) {
        out += "[";
        for (const auto &t : audio_tracks) {
            out += fmt::format("ID {}: Type: Audio, Sample Rate: {}, "
                               "Channels: {}, Bit Rate: {}",
                               t->track_id, t->properties.audio.sample_rate,
                               t->properties.audio.channels,
                               t->properties.audio.bit_rate);
        }
        out += "] ";
    }
    if (!video_tracks.empty()) {
        out += "[";
        for (const auto &t : video_tracks) {
            out += fmt::format(
                "ID {}: Type: Video, Width: {}, Height: {}, Frame Rate: "
                "{}, Bit Rate: {}",
                t->track_id, t->properties.video.width,
                t->properties.video.height, t->properties.video.frame_rate,
                t->properties.video.bit_rate);
        }
        out += "] ";
    }
    if (!subtitle_tracks.empty()) {
        for (const auto &t : subtitle_tracks) {
            out += fmt::format("ID {}: Type: Subtitle", t->track_id);
        }
    }
    if (!unknown_tracks.empty()) {
        out += "[";
        for (const auto &t : unknown_tracks) {
            out += fmt::format("ID: {}, Type: Unknown", t->track_id);
        }
        out += "] ";
    }

    return out;
}

} // namespace yapl
