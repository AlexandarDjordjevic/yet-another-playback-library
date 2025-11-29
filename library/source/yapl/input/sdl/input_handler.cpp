#include "yapl/input/sdl/input_handler.hpp"
#include "yapl/detail/debug.hpp"

#include <SDL2/SDL.h>

namespace yapl::input::sdl {

void input_handler::poll() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (!m_callback)
            continue;

        if (event.type == SDL_QUIT) {
            m_callback(command::quit);
            continue;
        }

        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_SPACE:
                m_callback(command::toggle_pause);
                break;
            case SDLK_q:
            case SDLK_ESCAPE:
                LOG_TRACE("SDL key: quit requested");
                m_callback(command::quit);
                break;
            case SDLK_RIGHT:
                m_callback(command::seek_forward);
                break;
            case SDLK_LEFT:
                m_callback(command::seek_backward);
                break;
            case SDLK_UP:
                m_callback(command::volume_up);
                break;
            case SDLK_DOWN:
                m_callback(command::volume_down);
                break;
            case SDLK_s:
                m_callback(command::show_stats);
                break;
            default:
                break;
            }
        }
    }
}

void input_handler::set_command_callback(command_callback callback) {
    m_callback = std::move(callback);
}

} // namespace yapl::input::sdl
