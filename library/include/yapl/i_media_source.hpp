#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace yapl {

struct i_media_source {
    virtual ~i_media_source() = default;
    virtual void open(const std::string_view url) = 0;
    virtual void close() = 0;
    virtual size_t read_packet(size_t size, std::span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
    virtual void reset() = 0;
};

} // namespace yapl
