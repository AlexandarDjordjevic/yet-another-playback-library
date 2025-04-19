#pragma once

#include "sklepan/debug.hpp"
#include "sklepan/Track.hpp"

#include <cstddef>
#include <vector>
#include <memory>


namespace sklepan {

struct MediaInfo {
    size_t numbersOfTracks;
    size_t duration;
    std::vector<std::shared_ptr<TrackInfo>> tracks;
};

static void inline printMediaInfo(const MediaInfo& mediaInfo) {
    LOG_INFO("Media info - Duration: {}, Number of tracks: {}", mediaInfo.duration, mediaInfo.numbersOfTracks);
    for (const auto& track : mediaInfo.tracks) {
        switch (track->type) {
            case TrackType::audio: {
                auto audioTrack = reinterpret_cast<AudioTrackInfo*>(track.get());
                LOG_INFO("{}. Audio track -> Sample Rate: {}, Channels: {}, Bit Rate: {}", 
                         audioTrack->trackId, audioTrack->sampleRate, audioTrack->channels, audioTrack->bitRate);
                break;
            }
            case TrackType::video: {
                auto videoTrack = reinterpret_cast<VideoTrackInfo*>(track.get());
                LOG_INFO("{}. Video track -> Width: {}, Height: {}, Frame Rate: {}, Bit Rate: {}", 
                         videoTrack->trackId, videoTrack->width, videoTrack->height, videoTrack->frameRate, videoTrack->bitRate);
                break;
            }
            case TrackType::subtitle: {
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
