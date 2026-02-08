#pragma once

#include "yapl/detail/blocking_queue.hpp"
#include "yapl/detail/sdl_resource_handles.hpp"
#include "yapl/renderers/i_audio_renderer.hpp"
#include "yapl/renderers/media_clock.hpp"

#include <SDL2/SDL.h>
#include <optional>

namespace yapl::renderers::sdl {

struct audio_renderer : i_audio_renderer {
    explicit audio_renderer(media_clock &clock);
    ~audio_renderer() override;

    void push_frame(std::shared_ptr<media_sample> frame) override;
    void render() override;
    void pause() override;
    void resume() override;
    void stop() override;
    [[nodiscard]] queue_stats get_queue_stats() const override;

  private:
    media_clock &m_clock;
    blocking_queue<std::shared_ptr<media_sample>> m_frames{60};
    std::optional<detail::sdl_audio_device_handle> m_audio_device;
    std::optional<std::shared_ptr<media_sample>> m_pending_frame;
};

} // namespace yapl::renderers::sdl
