#pragma once

#include "yapl/input/i_input_handler_factory.hpp"
#include "yapl/input/sdl/input_handler.hpp"

namespace yapl::input::sdl {

struct input_handler_factory : i_input_handler_factory {
    std::unique_ptr<i_input_handler> create() override {
        return std::make_unique<input_handler>();
    }
};

} // namespace yapl::input::sdl
