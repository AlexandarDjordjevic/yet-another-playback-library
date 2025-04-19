#pragma once
#include "MediaInfo.hpp"

#include <string>
#include <cstddef>
#include <cstdint>
#include <span>

namespace sklepan
{
struct IMediaSource {
    virtual ~IMediaSource() = default;
    virtual void open(const std::string_view url) = 0;
    virtual void close() = 0;
    virtual size_t readPacket(size_t size, std::span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
};

} // namespace sklepan
