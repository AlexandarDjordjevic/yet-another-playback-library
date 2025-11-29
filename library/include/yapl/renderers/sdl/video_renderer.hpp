#pragma once

#include "yapl/detail/blocking_queue.hpp"
#include "yapl/renderers/i_video_renderer.hpp"

#include <SDL2/SDL.h>
#include <atomic>
#include <cstddef>
#include <optional>

namespace yapl::renderers::sdl {

struct video_renderer : i_video_renderer {
    video_renderer();
    ~video_renderer() override;

    void resize(size_t width, size_t height) override;
    void push_frame(std::shared_ptr<media_sample> frame) override;
    void pause() override;
    void resume() override;
    void stop() override;
    void render() override;
    [[nodiscard]] queue_stats get_queue_stats() const override;
    [[nodiscard]] int64_t get_current_position_ms() const override;

  private:
    size_t m_width;
    size_t m_height;
    std::atomic<int64_t> m_current_position_ms{0};
    std::optional<std::shared_ptr<media_sample>> m_pending_frame;
    blocking_queue<std::shared_ptr<media_sample>> m_frames{60};
    SDL_Window *m_window{nullptr};
    SDL_Renderer *m_renderer{nullptr};
    SDL_Texture *m_texture{nullptr};
};

} // namespace yapl::renderers::sdl
