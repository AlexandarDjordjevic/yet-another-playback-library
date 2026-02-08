#pragma once

#include "yapl/renderers/i_video_renderer.hpp"
#include "yapl/renderers/media_clock.hpp"

#include <memory>

namespace yapl::renderers {

struct i_video_renderer_factory {
    virtual std::unique_ptr<i_video_renderer>
    create_video_renderer(media_clock &clock) = 0;
    virtual ~i_video_renderer_factory() = default;
};

} // namespace yapl::renderers
