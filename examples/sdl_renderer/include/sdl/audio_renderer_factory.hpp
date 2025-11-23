#pragma once

#include "sdl/audio_renderer.hpp"
#include "yapl/renderers/i_audio_renderer_factory.hpp"

#include <memory>

namespace renderers::sdl {

struct audio_renderer_factory : yapl::renderers::i_audio_renderer_factory {
    std::unique_ptr<yapl::renderers::i_audio_renderer> create_audio_renderer() {
        return std::make_unique<audio_renderer>();
    }
};

}; // namespace renderers::sdl
