#pragma once

#include "yapl/media_sample.hpp"
#include "yapl/pipeline_stats.hpp"
#include <memory>

namespace yapl::renderers {

struct i_video_renderer {
    virtual ~i_video_renderer() = default;

    virtual void resize(size_t width, size_t height) = 0;
    virtual void push_frame(std::shared_ptr<media_sample> frame) = 0;
    virtual void render() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    [[nodiscard]] virtual queue_stats get_queue_stats() const = 0;
    [[nodiscard]] virtual int64_t get_current_position_ms() const = 0;
};

} // namespace yapl::renderers
