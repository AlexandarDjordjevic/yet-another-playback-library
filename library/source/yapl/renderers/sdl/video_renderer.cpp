#include "yapl/renderers/sdl/video_renderer.hpp"

#include "SDL_render.h"
#include "yapl/debug.hpp"
#include <cstdio>
#include <cstdlib>

using namespace std::chrono_literals;

namespace yapl::renderers::sdl {

video_renderer::video_renderer()
    : m_width{640}, m_height{480}, m_running{false}, m_decoder_drained{false} {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw("Failed to Initialize SDL library!");
    }

    m_window =
        SDL_CreateWindow("YAPL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            static_cast<int>(m_width), static_cast<int>(m_height), SDL_WINDOW_SHOWN);

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    m_texture =
        SDL_CreateTexture(m_renderer,
                          SDL_PIXELFORMAT_IYUV, // YUV420P
                          SDL_TEXTUREACCESS_STREAMING, static_cast<int>(m_width), static_cast<int>(m_height));
}

video_renderer::~video_renderer() {
    m_running = false;
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    m_worker_thread.join();
}

void video_renderer::resize(size_t width, size_t height) {
    m_width = width;
    m_height = height;

    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);

    m_window =
        SDL_CreateWindow("YAPL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         static_cast<int>(m_width), static_cast<int>(m_height), SDL_WINDOW_SHOWN);

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    m_texture =
        SDL_CreateTexture(m_renderer,
                          SDL_PIXELFORMAT_IYUV, // YUV420P
                          SDL_TEXTUREACCESS_STREAMING, static_cast<int>(m_width), static_cast<int>(m_height));

}

void video_renderer::push_frame(std::shared_ptr<media_sample> frame) {

    m_frames.push(frame);
}

void video_renderer::stop() { m_running = false; }

void video_renderer::set_decoder_drained() { m_decoder_drained = true; }

void video_renderer::render() {
    m_running = true;
    SDL_Event event;

    int y_pitch = static_cast<int>(m_width);
    int u_pitch = static_cast<int>(m_width) / 2;
    int v_pitch = static_cast<int>(m_width) / 2;

    while (m_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                exit(EXIT_SUCCESS);
            }
        }
        SDL_Delay(30);

        if (m_frames.is_empty()) {
            m_running = !m_decoder_drained;
            continue;
        }

        // Pointers to Y, U, V planes
        auto result = m_frames.pop();
        uint8_t *y_plane = result->data.data();
        uint8_t *u_plane = y_plane + m_width * m_height;
        uint8_t *v_plane = u_plane + (m_width * m_height) / 4;
        // Update texture with YUV data
        SDL_UpdateYUVTexture(m_texture, nullptr, y_plane, y_pitch, u_plane,
                             u_pitch, v_plane, v_pitch);
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }
}

} // namespace yapl::renderers::sdl
