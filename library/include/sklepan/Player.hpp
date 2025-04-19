#pragma once

#include "sklepan/MediaPipeline.hpp"

#include <functional>
#include <memory>


namespace sklepan
{
    
class Player
{
public:
    Player();

    ~Player() = default;

    void load(const std::string_view url);
    void play();
    void pause();
    void stop();
    void seek(float position);
    void setVolume(float volume);
    void registerBufferUpdateCallback(std::function<void(size_t, size_t)> callback);
private:
    MediaInfo _mediaInfo;
    MediaPipeline _mediaPipeline;
};

} // namespace sklepan
