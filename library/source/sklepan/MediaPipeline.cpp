#include "sklepan/MediaPipeline.hpp"

#include "sklepan/MediaSource.hpp"
#include "sklepan/FFMpegMediaExtractor.hpp"
#include "sklepan/debug.hpp"

#include <algorithm>

namespace sklepan {

MediaPipeline::MediaPipeline(){
    _mediaSource = std::make_unique<MediaSource>();
    if (!_mediaSource) {
        throw std::runtime_error("Media source is null");
    }
    
    _mediaExtractor = std::make_unique<FFMpegMediaExtractor>(_mediaSource);
    if (!_mediaExtractor) {
        throw std::runtime_error("Media extractor is null");
    }

    _sampleReader = std::thread([this]() {
        while (true) {
            if (!this->_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            auto sample = _mediaExtractor->readSample();
            LOG_DEBUG("MediaPipeline - Sample read: trackId: {}, pts: {}, dts: {}", 
                      sample->trackId, sample->pts, sample->dts);
            _tracks[sample->trackId]->pushSample(sample);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    _sampleDecoder = std::thread([this]() {
        while (true) {
            if (!this->_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            auto sample = _tracks[0]->popSample();
            LOG_DEBUG("MediaPipeline - Sample decoded: trackId: {}, pts: {}, dts: {}", 
                      sample->trackId, sample->pts, sample->dts);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    LOG_INFO("MediaPipeline initialized");
}

MediaPipeline::~MediaPipeline() {
    if (_sampleReader.joinable()) {
        _sampleReader.join();
    }
    LOG_INFO("MediaPipeline destructor called");
}

void MediaPipeline::load(const std::string_view url) {
    LOG_INFO("MediaPipeline loading URL: {}", std::string(url));
    _mediaSource->open(url);
    _mediaExtractor->start();
    _mediaInfo = _mediaExtractor->getMediaInfo();
    std::ranges::for_each(_mediaInfo.tracks, [&](const auto& trackInfo) {
        LOG_DEBUG("MediaPipeline - Track ID: {}, Type: {}", trackInfo->trackId, static_cast<int>(trackInfo->type));
        _tracks.emplace_back(std::make_shared<Track>(trackInfo));
    });
}

void MediaPipeline::play() {
    LOG_INFO("MediaPipeline playing");
    _running = true;
    
}

void MediaPipeline::pause() {
    // Implement pause logic
}

void MediaPipeline::stop() {
    // Implement stop logic
}

void MediaPipeline::seek(float position) {
    // Implement seek logic
}

MediaInfo MediaPipeline::getMediaInfo() const {
    return _mediaInfo;
}

void MediaPipeline::registerBufferUpdateCallback(std::function<void(size_t, size_t)> callback){

}

} // namespace sklepan
