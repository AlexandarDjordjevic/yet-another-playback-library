#include "sklepan/Player.hpp"
#include "sklepan/debug.hpp"

#include "sklepan/FFMpegMediaExtractor.hpp"

namespace sklepan
{

Player::Player() {
    LOG_INFO("Player initialized");  
}

void Player::load(const std::string_view url) {
    LOG_INFO("Loading media from URL: {}", std::string(url));
    _mediaPipeline.load(url);
    _mediaInfo = _mediaPipeline.getMediaInfo();
    printMediaInfo(_mediaInfo);
}

void Player::play() {
    LOG_INFO("Playing media");
    _mediaPipeline.play();
}

void Player::pause() {
    // Implementation for pausing media
}
void Player::stop() {
    // Implementation for stopping media
}

void Player::seek(float position) {
    // Implementation for seeking to a specific position
}

void Player::setVolume(float volume) {
    // Implementation for setting volume
}

void Player::registerBufferUpdateCallback(std::function<void(size_t, size_t)> callback) {
    _mediaPipeline.registerBufferUpdateCallback(callback);
}

} // namespace sklepan
