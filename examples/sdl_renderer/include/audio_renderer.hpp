#pragma once

#include "yapl/blocking_queue.hpp"
#include "yapl/renderers/i_audio_renderer.hpp"

#include <SDL2/SDL.h>

namespace yapl::renderers::sdl {

struct audio_renderer : i_audio_renderer {
    audio_renderer();
    void push_frame(std::shared_ptr<media_sample>) override;
    void render() override;
    ~audio_renderer() override;

  private:
    data_queue<std::shared_ptr<media_sample>> m_frames{60};
    std::atomic_bool m_running;
    std::atomic_bool m_decoder_drained;
    std::thread m_worker_thread;
    SDL_AudioDeviceID m_audio_device;
};

} // namespace yapl::renderers::sdl
