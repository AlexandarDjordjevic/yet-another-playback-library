#pragma once

#include "yapl/renderers/i_video_renderer_factory.hpp"
#include "yapl/renderers/sdl/video_renderer.hpp"

namespace yapl::renderers::sdl {

struct video_renderer_factory : i_video_renderer_factory {
    std::unique_ptr<i_video_renderer>
    create_video_renderer(media_clock &clock, size_t queue_size) override {
        return std::make_unique<video_renderer>(clock, queue_size);
    }
};

} // namespace yapl::renderers::sdl
