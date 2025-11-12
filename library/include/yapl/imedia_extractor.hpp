#pragma once

#include "yapl/media_info.hpp"
#include "yapl/media_sample.hpp"

namespace yapl {

struct imedia_extractor {
    virtual ~imedia_extractor() = default;
    virtual void start() = 0;
    virtual std::shared_ptr<media_info> get_media_info() const = 0;
    virtual std::shared_ptr<media_sample> read_sample() = 0;

    // virtual void seek(std::chrono::seconds position) = 0;
    // virtual std::chrono::seconds getDuration() const = 0;
    // virtual size_t getSampleRate() const = 0;
    // virtual size_t getChannels() const = 0;
    // virtual size_t getBitRate() const = 0;
};

} // namespace yapl
