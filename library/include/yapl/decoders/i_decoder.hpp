#pragma once

#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"
#include <memory>

namespace yapl::decoders {

struct i_decoder {
    virtual ~i_decoder() = default;
    virtual bool decode(std::shared_ptr<track_info> info,
                        std::shared_ptr<media_sample> sample,
                        std::shared_ptr<media_sample> decoded_sample) = 0;
};

} // namespace yapl::decoders
