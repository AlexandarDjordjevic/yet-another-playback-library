#pragma once

#include "yapl/renderers/i_audio_renderer.hpp"
#include "yapl/renderers/media_clock.hpp"

#include <memory>

namespace yapl::renderers {

struct i_audio_renderer_factory {
    virtual std::unique_ptr<i_audio_renderer>
    create_audio_renderer(media_clock &clock, size_t queue_size) = 0;
    virtual ~i_audio_renderer_factory() = default;
};

} // namespace yapl::renderers
