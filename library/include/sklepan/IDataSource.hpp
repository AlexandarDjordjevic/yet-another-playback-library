#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

namespace sklepan{
 
struct IDataSource{
    virtual void open() = 0;
    virtual void close() = 0;
    virtual size_t readData(size_t size, std::span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
    virtual bool isOpen() const = 0;
    virtual ~IDataSource() = default;
};

} // namespace sklepan
