#pragma once

#include <SDL2/SDL.h>
#include <stdexcept>
#include <string>

namespace yapl::detail {

/**
 * @brief RAII wrapper for SDL_Window
 *
 * Automatically destroys SDL window on scope exit, preventing resource leaks.
 * Move-only type for unique ownership semantics.
 */
class sdl_window_handle {
  public:
    /**
     * @brief Create SDL window
     * @param title Window title
     * @param x Window X position (or SDL_WINDOWPOS_CENTERED)
     * @param y Window Y position (or SDL_WINDOWPOS_CENTERED)
     * @param w Window width in pixels
     * @param h Window height in pixels
     * @param flags SDL window flags (e.g., SDL_WINDOW_SHOWN)
     * @throws std::runtime_error if window creation fails
     */
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

    /**
     * @brief Get underlying SDL_Window pointer
     * @return SDL_Window pointer for passing to SDL functions
     */
    [[nodiscard]] SDL_Window *get() const { return m_window; }

  private:
    SDL_Window *m_window{nullptr};
};

/**
 * @brief RAII wrapper for SDL_Renderer
 *
 * Automatically destroys SDL renderer on scope exit. Move-only.
 */
class sdl_renderer_handle {
  public:
    /**
     * @brief Create SDL renderer
     * @param window Window to create renderer for
     * @param index Driver index (-1 for first supporting flags)
     * @param flags SDL renderer flags
     * @throws std::runtime_error if renderer creation fails
     */
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

    /** @brief Get underlying SDL_Renderer pointer */
    [[nodiscard]] SDL_Renderer *get() const { return m_renderer; }

  private:
    SDL_Renderer *m_renderer{nullptr};
};

/**
 * @brief RAII wrapper for SDL_Texture
 *
 * Automatically destroys SDL texture on scope exit. Move-only.
 */
class sdl_texture_handle {
  public:
    /**
     * @brief Create SDL texture
     * @param renderer Renderer to create texture for
     * @param format Pixel format (e.g., SDL_PIXELFORMAT_IYUV)
     * @param access Texture access pattern
     * @param w Texture width in pixels
     * @param h Texture height in pixels
     * @throws std::runtime_error if texture creation fails
     */
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

    /** @brief Get underlying SDL_Texture pointer */
    [[nodiscard]] SDL_Texture *get() const { return m_texture; }

  private:
    SDL_Texture *m_texture{nullptr};
};

/**
 * @brief RAII wrapper for SDL_AudioDeviceID
 *
 * Automatically closes SDL audio device on scope exit. Move-only.
 */
class sdl_audio_device_handle {
  public:
    /**
     * @brief Open SDL audio device
     * @param device Device name (nullptr for default)
     * @param iscapture 0 for playback, non-zero for recording
     * @param desired Desired audio spec
     * @param obtained Actual audio spec (may differ from desired)
     * @param allowed_changes Flags for allowed spec changes
     * @throws std::runtime_error if device open fails
     */
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

    sdl_audio_device_handle(const sdl_audio_device_handle &) = delete;
    sdl_audio_device_handle &operator=(const sdl_audio_device_handle &) = delete;

    sdl_audio_device_handle(sdl_audio_device_handle &&other) noexcept
        : m_device_id{other.m_device_id} {
        other.m_device_id = 0;
    }

    sdl_audio_device_handle &operator=(sdl_audio_device_handle &&other) noexcept {
        if (this != &other) {
            if (m_device_id) {
                SDL_CloseAudioDevice(m_device_id);
            }
            m_device_id = other.m_device_id;
            other.m_device_id = 0;
        }
        return *this;
    }

    /** @brief Get underlying SDL_AudioDeviceID */
    [[nodiscard]] SDL_AudioDeviceID get() const { return m_device_id; }

  private:
    SDL_AudioDeviceID m_device_id{0};
};

} // namespace yapl::detail
