#pragma once

#include "yapl/renderers/i_audio_renderer_factory.hpp"
#include "yapl/renderers/sdl/audio_renderer.hpp"

namespace yapl::renderers::sdl {

struct audio_renderer_factory : i_audio_renderer_factory {
    std::unique_ptr<i_audio_renderer>
    create_audio_renderer(media_clock &clock, size_t queue_size) override {
        return std::make_unique<audio_renderer>(clock, queue_size);
    }
};

} // namespace yapl::renderers::sdl
