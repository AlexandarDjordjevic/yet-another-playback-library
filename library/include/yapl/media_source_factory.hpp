#pragma once

#include "yapl/i_media_source_factory.hpp"
#include "yapl/detail/media_source.hpp"

namespace yapl {

struct media_source_factory : i_media_source_factory {
    std::shared_ptr<i_media_source> create() override {
        return std::make_shared<media_source>();
    }
};

} // namespace yapl
