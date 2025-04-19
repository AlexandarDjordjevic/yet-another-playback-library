#pragma once

#include "IMediaSource.hpp"
#include "IMediaExtractor.hpp"
#include "MediaInfo.hpp"
#include "Track.hpp"

#include <memory>


namespace sklepan {

class MediaPipeline{
public:
    MediaPipeline();
    ~MediaPipeline();
    
    void load(const std::string_view url);
    void play();
    void pause();
    void stop();
    void seek(float position);
    MediaInfo getMediaInfo() const;
    void registerBufferUpdateCallback(std::function<void(size_t, size_t)> callback);
    
private:
    bool _running;
    std::thread _sampleReader;
    std::thread _sampleDecoder;
    MediaInfo _mediaInfo;
    std::shared_ptr<IMediaSource> _mediaSource;
    std::unique_ptr<IMediaExtractor> _mediaExtractor;
    std::vector<std::shared_ptr<Track>> _tracks;
};

} // namespace sklepan
