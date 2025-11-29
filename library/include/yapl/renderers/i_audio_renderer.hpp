#pragma once

#include "yapl/media_sample.hpp"
#include "yapl/pipeline_stats.hpp"
#include <memory>

namespace yapl::renderers {

struct i_audio_renderer {
    virtual ~i_audio_renderer() = default;

    virtual void push_frame(std::shared_ptr<media_sample> frame) = 0;
    virtual void render() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    [[nodiscard]] virtual queue_stats get_queue_stats() const = 0;
};

} // namespace yapl::renderers
