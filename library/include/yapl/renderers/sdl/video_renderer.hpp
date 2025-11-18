#pragma once

#include "yapl/renderers/ivideo_renderer.hpp"

#include <SDL2/SDL.h>
#include <atomic>
#include <cstddef>
#include <map>
#include <thread>

namespace yapl::renderers::sdl {

struct video_renderer : ivideo_renderer {
    video_renderer(size_t, size_t);
    ~video_renderer();
    void push_frame(std::shared_ptr<media_sample>) override;
    void stop() override;
    void render() override;
    void set_decoder_drained() override;

  private:
    size_t m_width;
    size_t m_height;
    std::map<int64_t, std::shared_ptr<media_sample>> m_frames;
    std::mutex m_frames_mtx;
    std::atomic_bool m_running;
    std::atomic_bool m_decoder_drained;
    std::thread m_worker_thread;
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;
    SDL_Texture *m_texture;
};

} // namespace yapl::renderers::sdl
