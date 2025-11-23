#pragma once

#include "sdl/video_renderer.hpp"
#include "yapl/renderers/i_video_renderer_factory.hpp"

#include <memory>

namespace renderers::sdl {

struct video_renderer_factory : yapl::renderers::i_video_renderer_factory {
    std::unique_ptr<yapl::renderers::i_video_renderer> create_video_renderer() {
        return std::make_unique<video_renderer>();
    }
};

} // namespace renderers::sdl
