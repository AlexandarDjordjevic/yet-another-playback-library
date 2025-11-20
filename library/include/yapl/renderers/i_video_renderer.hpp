#pragma once

#include "yapl/media_sample.hpp"
#include <memory>

namespace yapl::renderers {

struct i_video_renderer {
    virtual void resize(size_t, size_t) = 0;
    virtual void push_frame(std::shared_ptr<media_sample>) = 0;
    virtual void render() = 0;
    virtual void stop() = 0;
    virtual void set_decoder_drained() = 0;
    virtual ~i_video_renderer() = default;
};

} // namespace yapl::renderers
