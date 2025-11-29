#pragma once

#include "yapl/input/i_input_handler.hpp"

namespace yapl::input::sdl {

struct input_handler : i_input_handler {
    input_handler() = default;
    ~input_handler() override = default;

    void poll() override;
    void set_command_callback(command_callback callback) override;

  private:
    command_callback m_callback;
};

} // namespace yapl::input::sdl
