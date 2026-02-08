#pragma once

#include <SDL2/SDL.h>
#include <stdexcept>
#include <string>

namespace yapl::detail {

/// RAII wrapper for SDL_Window
class sdl_window_handle {
  public:
    sdl_window_handle(const char *title, int x, int y, int w, int h,
                      Uint32 flags) {
        m_window = SDL_CreateWindow(title, x, y, w, h, flags);
        if (!m_window) {
            throw std::runtime_error(std::string("SDL_CreateWindow failed: ") +
                                     SDL_GetError());
        }
    }

    ~sdl_window_handle() {
        if (m_window) {
            SDL_DestroyWindow(m_window);
        }
    }

    // Non-copyable, movable
    sdl_window_handle(const sdl_window_handle &) = delete;
    sdl_window_handle &operator=(const sdl_window_handle &) = delete;
    sdl_window_handle(sdl_window_handle &&other) noexcept
        : m_window{other.m_window} {
        other.m_window = nullptr;
    }
    sdl_window_handle &operator=(sdl_window_handle &&other) noexcept {
        if (this != &other) {
            if (m_window) {
                SDL_DestroyWindow(m_window);
            }
            m_window = other.m_window;
            other.m_window = nullptr;
        }
        return *this;
    }

    [[nodiscard]] SDL_Window *get() const { return m_window; }

  private:
    SDL_Window *m_window{nullptr};
};

/// RAII wrapper for SDL_Renderer
class sdl_renderer_handle {
  public:
    sdl_renderer_handle(SDL_Window *window, int index, Uint32 flags) {
        m_renderer = SDL_CreateRenderer(window, index, flags);
        if (!m_renderer) {
            throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") +
                                     SDL_GetError());
        }
    }

    ~sdl_renderer_handle() {
        if (m_renderer) {
            SDL_DestroyRenderer(m_renderer);
        }
    }

    // Non-copyable, movable
    sdl_renderer_handle(const sdl_renderer_handle &) = delete;
    sdl_renderer_handle &operator=(const sdl_renderer_handle &) = delete;
    sdl_renderer_handle(sdl_renderer_handle &&other) noexcept
        : m_renderer{other.m_renderer} {
        other.m_renderer = nullptr;
    }
    sdl_renderer_handle &operator=(sdl_renderer_handle &&other) noexcept {
        if (this != &other) {
            if (m_renderer) {
                SDL_DestroyRenderer(m_renderer);
            }
            m_renderer = other.m_renderer;
            other.m_renderer = nullptr;
        }
        return *this;
    }

    [[nodiscard]] SDL_Renderer *get() const { return m_renderer; }

  private:
    SDL_Renderer *m_renderer{nullptr};
};

/// RAII wrapper for SDL_Texture
class sdl_texture_handle {
  public:
    sdl_texture_handle(SDL_Renderer *renderer, Uint32 format, int access, int w,
                       int h) {
        m_texture = SDL_CreateTexture(renderer, format, access, w, h);
        if (!m_texture) {
            throw std::runtime_error(std::string("SDL_CreateTexture failed: ") +
                                     SDL_GetError());
        }
    }

    ~sdl_texture_handle() {
        if (m_texture) {
            SDL_DestroyTexture(m_texture);
        }
    }

    // Non-copyable, movable
    sdl_texture_handle(const sdl_texture_handle &) = delete;
    sdl_texture_handle &operator=(const sdl_texture_handle &) = delete;
    sdl_texture_handle(sdl_texture_handle &&other) noexcept
        : m_texture{other.m_texture} {
        other.m_texture = nullptr;
    }
    sdl_texture_handle &operator=(sdl_texture_handle &&other) noexcept {
        if (this != &other) {
            if (m_texture) {
                SDL_DestroyTexture(m_texture);
            }
            m_texture = other.m_texture;
            other.m_texture = nullptr;
        }
        return *this;
    }

    [[nodiscard]] SDL_Texture *get() const { return m_texture; }

  private:
    SDL_Texture *m_texture{nullptr};
};

/// RAII wrapper for SDL_AudioDeviceID
class sdl_audio_device_handle {
  public:
    sdl_audio_device_handle(const char *device, int iscapture,
                            const SDL_AudioSpec *desired, SDL_AudioSpec *obtained,
                            int allowed_changes) {
        m_device_id =
            SDL_OpenAudioDevice(device, iscapture, desired, obtained, allowed_changes);
        if (!m_device_id) {
            throw std::runtime_error(std::string("SDL_OpenAudioDevice failed: ") +
                                     SDL_GetError());
        }
    }

    ~sdl_audio_device_handle() {
        if (m_device_id) {
            SDL_CloseAudioDevice(m_device_id);
        }
    }

    // Non-copyable, movable
    sdl_audio_device_handle(const sdl_audio_device_handle &) = delete;
    sdl_audio_device_handle &operator=(const sdl_audio_device_handle &) = delete;
    sdl_audio_device_handle(sdl_audio_device_handle &&other) noexcept
        : m_device_id{other.m_device_id} {
        other.m_device_id = 0;
    }
    sdl_audio_device_handle &
    operator=(sdl_audio_device_handle &&other) noexcept {
        if (this != &other) {
            if (m_device_id) {
                SDL_CloseAudioDevice(m_device_id);
            }
            m_device_id = other.m_device_id;
            other.m_device_id = 0;
        }
        return *this;
    }

    [[nodiscard]] SDL_AudioDeviceID get() const { return m_device_id; }

  private:
    SDL_AudioDeviceID m_device_id{0};
};

} // namespace yapl::detail
