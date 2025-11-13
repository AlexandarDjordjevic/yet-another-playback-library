#include "yapl/player.hpp"

#include "yapl/debug.hpp"
#include "yapl/media_info.hpp"

namespace yapl {

player::player() { LOG_INFO("player initialized"); }

void player::load(const std::string_view url) {
    LOG_DEBUG("Player -> loading media from URL: {}", std::string(url));
    m_media_pipeline.load(url);
    LOG_DEBUG("{}", to_string(m_media_pipeline.get_media_info()));
}

void player::play() { m_media_pipeline.play(); }

void player::pause() {
    // Implementation for pausing media
}
void player::stop() {
    // Implementation for stopping media
}

void player::seek([[maybe_unused]] float position) {
    // Implementation for seeking to a specific position
}

void player::set_volume([[maybe_unused]] float volume) {
    // Implementation for setting volume
}

void player::register_buffer_update_handler(
    [[maybe_unused]] std::function<void(size_t, size_t)> callback) {
    // _mediaPipeline.register_buffer_update_handler(callback);
}

} // namespace yapl
