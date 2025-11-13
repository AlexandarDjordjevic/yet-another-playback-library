#include "yapl/renderers/sdl/video_renderer.hpp"
#include "SDL_render.h"
#include "yapl/debug.hpp"
#include <cstdio>
#include <cstdlib>
#include <mutex>

using namespace std::chrono_literals;

namespace yapl::renderers::sdl {

video_renderer::video_renderer(size_t width, size_t height)
    : m_width{width}, m_height{height} {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw("Failed to Initialzie SDL library!");
    }

    m_window =
        SDL_CreateWindow("YAPL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         m_width, m_height, SDL_WINDOW_SHOWN);

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    m_texture =
        SDL_CreateTexture(m_renderer,
                          SDL_PIXELFORMAT_IYUV, // YUV420P
                          SDL_TEXTUREACCESS_STREAMING, m_width, m_height);
}

video_renderer::~video_renderer() {
    m_running = false;
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    m_worker_thread.join();
}

void video_renderer::push_frame(std::shared_ptr<media_sample> frame) {
    std::scoped_lock<std::mutex> lock{m_frames_mtx};
    LOG_INFO("Renderer push frame. Frame size: {}", frame->data.size());
    m_frames.insert({frame->pts, frame});
}

void video_renderer::render() {
    m_running = true;
    SDL_Event event;

    int y_pitch = m_width;
    int u_pitch = m_width / 2;
    int v_pitch = m_width / 2;

    while (m_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                m_running = false;
        }
        SDL_Delay(30);

        std::this_thread::sleep_for(30ms);
        if (m_frames.empty()) {
            continue;
        }

        {
            std::scoped_lock<std::mutex> lock{m_frames_mtx};
            // Pointers to Y, U, V planes

            uint8_t *y_plane = m_frames.begin()->second->data.data();
            uint8_t *u_plane = y_plane + m_width * m_height;
            uint8_t *v_plane = u_plane + (m_width * m_height) / 4;
            // Update texture with YUV data
            SDL_UpdateYUVTexture(m_texture, nullptr, y_plane, y_pitch, u_plane,
                                 u_pitch, v_plane, v_pitch);
            SDL_RenderClear(m_renderer);
            SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
            SDL_RenderPresent(m_renderer);

            m_frames.erase(m_frames.begin());
        }
    }
}

} // namespace yapl::renderers::sdl
