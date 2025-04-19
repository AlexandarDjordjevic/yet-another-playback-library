#pragma once

#include <cstddef>
#include <vector>
#include <memory>
#include <format>

#include "sklepan/debug.hpp"

namespace sklepan {

enum class TrackType {
    AUDIO,
    VIDEO,
    SUBTITLE
};

struct Track {
    size_t trackId;
    TrackType type;
};

struct AudioTrackInfo : Track {
    size_t sampleRate;
    size_t channels;
    size_t bitRate;
};

struct VideoTrackInfo : Track {
    size_t width;
    size_t height;
    size_t frameRate;
    size_t bitRate;
};

struct MediaInfo {
    size_t numbersOfTracks;
    size_t duration;
    std::vector<std::shared_ptr<Track>> tracks;
};

static void inline printMediaInfo(const MediaInfo& mediaInfo) {
    LOG_INFO("Media info - Duration: {}, Number of tracks: {}", mediaInfo.duration, mediaInfo.numbersOfTracks);
    for (const auto& track : mediaInfo.tracks) {
        switch (track->type) {
            case TrackType::AUDIO: {
                auto audioTrack = reinterpret_cast<AudioTrackInfo*>(track.get());
                LOG_INFO("{}. Audio track -> Sample Rate: {}, Channels: {}, Bit Rate: {}", 
                         audioTrack->trackId, audioTrack->sampleRate, audioTrack->channels, audioTrack->bitRate);
                break;
            }
            case TrackType::VIDEO: {
                auto videoTrack = reinterpret_cast<VideoTrackInfo*>(track.get());
                LOG_INFO("{}. Video track -> Width: {}, Height: {}, Frame Rate: {}, Bit Rate: {}", 
                         videoTrack->trackId, videoTrack->width, videoTrack->height, videoTrack->frameRate, videoTrack->bitRate);
                break;
            }
            case TrackType::SUBTITLE: {
                LOG_INFO("{}. Subtitle track\n", track->trackId);
                break;
            }
            default:
                LOG_WARN("Unknown track type - ID: {}", track->trackId);
                break;
        }
    }
}

} // namespace sklepan
