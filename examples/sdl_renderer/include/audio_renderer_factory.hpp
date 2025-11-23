#pragma once

#include "yapl/renderers/i_audio_renderer_factory.hpp"
#include "yapl/renderers/sdl/audio_renderer.hpp"

#include <memory>

namespace yapl::renderers::sdl {

struct audio_renderer_factory : i_audio_renderer_factory {
    std::unique_ptr<i_audio_renderer> create_audio_renderer() {
        return std::make_unique<audio_renderer>();
    }
};

}; // namespace yapl::renderers::sdl
