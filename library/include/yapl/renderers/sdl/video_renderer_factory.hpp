#pragma once

#include "yapl/renderers/i_video_renderer_factory.hpp"
#include "yapl/renderers/sdl/video_renderer.hpp"

#include <memory>

namespace yapl::renderers::sdl {

struct video_renderer_factory : i_video_renderer_factory {
    std::unique_ptr<i_video_renderer> create_video_renderer() {
        return std::make_unique<video_renderer>();
    }
};

}; // namespace yapl::renderers::sdl
