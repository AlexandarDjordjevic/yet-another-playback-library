#pragma once

#include "yapl/detail/blocking_queue.hpp"
#include "yapl/renderers/i_audio_renderer.hpp"

#include <SDL2/SDL.h>
#include <optional>

namespace yapl::renderers::sdl {

struct audio_renderer : i_audio_renderer {
    audio_renderer();
    ~audio_renderer() override;

    void push_frame(std::shared_ptr<media_sample> frame) override;
    void render() override;
    void pause() override;
    void resume() override;
    void stop() override;
    [[nodiscard]] queue_stats get_queue_stats() const override;

  private:
    blocking_queue<std::shared_ptr<media_sample>> m_frames{60};
    SDL_AudioDeviceID m_audio_device{0};
    std::optional<std::shared_ptr<media_sample>> m_pending_frame;
};

} // namespace yapl::renderers::sdl
