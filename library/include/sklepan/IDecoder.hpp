#pragma once

#include <cstdint>
#include <span>

namespace sklepan {

struct IDecoder {
    virtual ~IDecoder() = default;
    virtual bool decodeData(std::span<uint8_t> inputBuffer, std::span<uint8_t> outputBuffer) = 0;
};


