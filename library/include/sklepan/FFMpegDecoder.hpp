#pragma once

#include "IDecoder.hpp"

namespace sklepan {

struct FFMpegDecoder : public IDecoder {
    FFMpegDecoder();
    ~FFMpegDecoder() override;

    bool decodeData(std::span<uint8_t> inputBuffer, std::span<uint8_t> outputBuffer) override;
};

} // namespace sklepan
