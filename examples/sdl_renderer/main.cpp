#include <memory>
#include <yapl/debug.hpp>
#include <yapl/player.hpp>

#include "sdl/audio_renderer_factory.hpp"
#include "sdl/video_renderer_factory.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        LOG_ERROR("Usage: \n\t\t {} <video_url>", argv[0]);
    }
    auto arf = std::make_unique<renderers::sdl::audio_renderer_factory>();
    auto vrf = std::make_unique<renderers::sdl::video_renderer_factory>();
    yapl::player p{std::move(vrf), std::move(arf)};
    p.load(argv[1]);
    p.play();

    return 0;
}
