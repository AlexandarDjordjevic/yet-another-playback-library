#pragma once

#include <functional>
#include <string_view>

namespace yapl::input {

enum class command {
    none,
    toggle_pause,
    quit,
    seek_forward,
    seek_backward,
    volume_up,
    volume_down,
    show_stats
};

constexpr std::string_view command_to_string(command cmd) noexcept {
    switch (cmd) {
    case command::none:
        return "none";
    case command::toggle_pause:
        return "toggle_pause";
    case command::quit:
        return "quit";
    case command::seek_forward:
        return "seek_forward";
    case command::seek_backward:
        return "seek_backward";
    case command::volume_up:
        return "volume_up";
    case command::volume_down:
        return "volume_down";
    case command::show_stats:
        return "show_stats";
    }
    return "unknown";
}

using command_callback = std::function<void(command)>;

struct i_input_handler {
    virtual ~i_input_handler() = default;
    
    // Poll for input events, call callback for each command
    virtual void poll() = 0;
    
    // Set callback for commands
    virtual void set_command_callback(command_callback callback) = 0;
};

} // namespace yapl::input
