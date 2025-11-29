#pragma once

#include "yapl/input/i_input_handler.hpp"
#include <memory>

namespace yapl::input {

struct i_input_handler_factory {
    virtual ~i_input_handler_factory() = default;
    virtual std::unique_ptr<i_input_handler> create() = 0;
};

} // namespace yapl::input
