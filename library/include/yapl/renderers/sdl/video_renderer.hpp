#pragma once

#include "yapl/detail/blocking_queue.hpp"
#include "yapl/detail/sdl_resource_handles.hpp"
#include "yapl/renderers/i_video_renderer.hpp"
#include "yapl/renderers/media_clock.hpp"

#include <SDL2/SDL.h>
#include <atomic>
#include <cstddef>
#include <optional>

namespace yapl::renderers::sdl {

struct video_renderer : i_video_renderer {
    explicit video_renderer(media_clock &clock);
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
    media_clock &m_clock;
    size_t m_width;
    size_t m_height;
    std::atomic<int64_t> m_current_position_ms{0};
    std::optional<std::shared_ptr<media_sample>> m_pending_frame;
    blocking_queue<std::shared_ptr<media_sample>> m_frames{60};
    std::optional<detail::sdl_window_handle> m_window;
    std::optional<detail::sdl_renderer_handle> m_renderer;
    std::optional<detail::sdl_texture_handle> m_texture;
};

} // namespace yapl::renderers::sdl
