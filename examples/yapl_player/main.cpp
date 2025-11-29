#include <yapl/detail/debug.hpp>
#include <yapl/input/i_input_handler.hpp>
#include <yapl/input/sdl/input_handler_factory.hpp>
#include <yapl/player.hpp>
#include <yapl/renderers/sdl/audio_renderer_factory.hpp>
#include <yapl/renderers/sdl/video_renderer_factory.hpp>

#include <memory>

int main(int argc, char **argv) {
    if (argc < 2) {
        LOG_ERROR("Usage: {} <media_file_or_url>", argv[0]);
        return 1;
    }

    // Create SDL-based factories for rendering and input
    auto video_factory =
        std::make_unique<yapl::renderers::sdl::video_renderer_factory>();
    auto audio_factory =
        std::make_unique<yapl::renderers::sdl::audio_renderer_factory>();
    auto input_factory =
        std::make_unique<yapl::input::sdl::input_handler_factory>();

    // Create the player with SDL backends
    yapl::player player{std::move(video_factory), std::move(audio_factory),
                        std::move(input_factory)};

    // Set up input command handling
    player.set_command_callback([&](yapl::input::command cmd) {
        switch (cmd) {
        case yapl::input::command::toggle_pause:
            if (player.is_paused()) {
                player.resume();
            } else {
                player.pause();
            }
            break;
        case yapl::input::command::quit:
            player.stop();
            break;
        case yapl::input::command::seek_forward:
            LOG_WARN("Seek forward not implemented");
            break;
        case yapl::input::command::seek_backward:
            LOG_WARN("Seek backward not implemented");
            break;
        case yapl::input::command::volume_up:
            LOG_WARN("Volume up not implemented");
            break;
        case yapl::input::command::volume_down:
            LOG_WARN("Volume down not implemented");
            break;
        case yapl::input::command::show_stats:
            LOG_INFO("Stats: {}", player.get_stats().to_string());
            break;
        default:
            break;
        }
    });

    // Load and play
    player.load(argv[1]);
    LOG_INFO("Controls: SPACE=Pause, S=Stats, Q/ESC=Quit");
    player.play();

    return 0;
}
