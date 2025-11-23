#pragma once

#include "yapl/blocking_queue.hpp"
#include "yapl/renderers/i_video_renderer.hpp"

#include <SDL2/SDL.h>
#include <atomic>
#include <cstddef>
#include <thread>

namespace renderers::sdl {

struct video_renderer : yapl::renderers::i_video_renderer {
    video_renderer();
    ~video_renderer();
    void resize(size_t, size_t) override;
    void push_frame(std::shared_ptr<yapl::media_sample>) override;
    void stop() override;
    void render() override;
    void set_decoder_drained() override;

  private:
    size_t m_width;
    size_t m_height;
    yapl::blocking_queue<std::shared_ptr<yapl::media_sample>> m_frames{60};
    std::atomic_bool m_running;
    std::atomic_bool m_decoder_drained;
    std::thread m_worker_thread;
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;
    SDL_Texture *m_texture;
};

} // namespace renderers::sdl
