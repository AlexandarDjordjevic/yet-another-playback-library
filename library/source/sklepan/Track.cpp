#include "sklepan/Track.hpp"
#include "sklepan/debug.hpp"

namespace sklepan {

Track::Track(const std::shared_ptr<TrackInfo> info) : _trackInfo(info), _sampleQueue{10}{}

void Track::pushSample(const std::shared_ptr<MediaSample> sample) {
    // Process the sample
    // For example, you can store it in a buffer or send it to a decoder
    // For now, we will just print the sample information
    LOG_DEBUG("Track - Sample pushed: trackId: {}, pts: {}, dts: {}", sample->trackId, sample->pts, sample->dts);
    _sampleQueue.push(sample);
}

std::shared_ptr<MediaSample> Track::popSample() {
    // Get the sample from the queue
    auto sample = _sampleQueue.pop();
    if (sample) {
        LOG_DEBUG("Track - Sample popped: trackId: {}, pts: {}, dts: {}", sample->trackId, sample->pts, sample->dts);
    } else {
        LOG_WARN("Track - No sample available");
    }
    return sample;
}

} // namespace sklepan
