#pragma once

#include "MediaSample.hpp"
#include "BlockingQueue.hpp"

#include <cstddef>
#include <memory>

namespace sklepan {

enum class TrackType {
    audio,
    video,
    subtitle
};

struct TrackInfo {
    size_t trackId;
    TrackType type;
};

struct AudioTrackInfo : TrackInfo {
    size_t sampleRate;
    size_t channels;
    size_t bitRate;
};

struct VideoTrackInfo : TrackInfo {
    size_t width;
    size_t height;
    size_t frameRate;
    size_t bitRate;
};

class Track {
public:
    Track(const std::shared_ptr<TrackInfo> info);
    void pushSample(const std::shared_ptr<MediaSample> sample);
    std::shared_ptr<MediaSample> popSample();
private:
    std::shared_ptr<TrackInfo> _trackInfo;
    BlockingQueue<std::shared_ptr<MediaSample>> _sampleQueue;
};

} // namespace sklepan
