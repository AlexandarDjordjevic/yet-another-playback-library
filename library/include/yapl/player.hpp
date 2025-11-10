#pragma once

#include "yapl/media_info.hpp"
#include "yapl/media_pipeline.hpp"

#include <functional>
#include <string_view>

namespace yapl {

class player {
  public:
    player();

    ~player() = default;

    void load(const std::string_view url);
    void play();
    void pause();
    void stop();
    void seek(float position);
    void set_volume(float volume);
    void register_buffer_update_handler(
        std::function<void(size_t, size_t)> callback);

  private:
    media_info m_media_info;
    media_pipeline m_media_pipeline;
};

} // namespace yapl
