#pragma once

#include "yapl/i_media_source.hpp"
#include <memory>

namespace yapl {

struct i_media_source_factory {
    virtual ~i_media_source_factory() = default;
    virtual std::shared_ptr<i_media_source> create() = 0;
};

} // namespace yapl
