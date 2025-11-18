#pragma once

#include "yapl/media_sample.hpp"
#include <memory>

namespace yapl::renderers {

struct ivideo_renderer {
    virtual void push_frame(std::shared_ptr<media_sample>) = 0;
    virtual void render() = 0;
    virtual void stop() = 0;
    virtual void set_decoder_drained() = 0;
};

} // namespace yapl::renderers
