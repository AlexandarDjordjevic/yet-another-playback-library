#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

namespace yapl {

struct idata_source {
    virtual void open() = 0;
    virtual void close() = 0;
    virtual size_t read_data(size_t size, std::span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
    virtual bool is_open() const = 0;
    virtual void reset() = 0;
    virtual ~idata_source() = default;
};

} // namespace yapl
