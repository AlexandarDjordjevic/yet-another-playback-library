#pragma once

#include "MediaInfo.hpp"
#include "MediaSample.hpp"

#include <cstddef>
#include <span>

namespace sklepan {

struct IMediaExtractor {
    virtual ~IMediaExtractor() = default;
    virtual void start() = 0;
    virtual MediaInfo getMediaInfo() = 0;
    virtual std::shared_ptr<MediaSample> readSample() = 0;

    // virtual void seek(std::chrono::seconds position) = 0;
    // virtual std::chrono::seconds getDuration() const = 0;
    // virtual size_t getSampleRate() const = 0;
    // virtual size_t getChannels() const = 0;
    // virtual size_t getBitRate() const = 0;
};

} // namespace sklepan
