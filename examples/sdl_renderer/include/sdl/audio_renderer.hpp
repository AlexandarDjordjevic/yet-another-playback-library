#pragma once

#include "yapl/blocking_queue.hpp"
#include "yapl/renderers/i_audio_renderer.hpp"

#include <SDL2/SDL.h>

namespace renderers::sdl {

struct audio_renderer : yapl::renderers::i_audio_renderer {
    audio_renderer();
    void push_frame(std::shared_ptr<yapl::media_sample>) override;
    void render() override;
    ~audio_renderer() override;

  private:
    yapl::blocking_queue<std::shared_ptr<yapl::media_sample>> m_frames{60};
    std::atomic_bool m_running;
    std::atomic_bool m_decoder_drained;
    std::thread m_worker_thread;
    SDL_AudioDeviceID m_audio_device;
};

} // namespace renderers::sdl
