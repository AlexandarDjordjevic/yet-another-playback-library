#pragma once

#include "yapl/media_info.hpp"
#include "yapl/media_pipeline.hpp"
#include "yapl/renderers/sdl/video_renderer.hpp"
#include "yapl/renderers/sdl/video_renderer_factory.hpp"

#include <functional>
#include <string_view>

namespace yapl {

class player {
  public:
    player(std::unique_ptr<renderers::i_video_renderer_factory> vrf, std::unique_ptr<renderers::i_audio_renderer_factory> arf);
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
    std::unique_ptr<media_pipeline> m_media_pipeline;
};

} // namespace yapl
