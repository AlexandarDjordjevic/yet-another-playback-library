#pragma once

#include "yapl/media_sample.hpp"

namespace yapl::renderers {

struct i_audio_renderer {
    virtual void push_frame(std::shared_ptr<media_sample>) = 0;
    virtual void render() = 0;
    virtual ~i_audio_renderer() = default;
};

} // namespace yapl::renderers
