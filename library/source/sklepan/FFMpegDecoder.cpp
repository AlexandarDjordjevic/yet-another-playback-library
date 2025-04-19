#include "sklepan/FFMpegDecoder.hpp"
#include "sklepan/debug.hpp"

namespace sklepan {

FFMpegDecoder::FFMpegDecoder() {
    // Initialize FFmpeg decoder
    LOG_INFO("FFMpegDecoder initialized");
}

FFMpegDecoder::~FFMpegDecoder() {
    // Cleanup FFmpeg decoder
    LOG_INFO("FFMpegDecoder destroyed");
}

bool FFMpegDecoder::decodeData(std::span<uint8_t> inputBuffer, std::span<uint8_t> outputBuffer) {
    // Decode data using FFmpeg
    LOG_INFO("Decoding data with FFMpegDecoder");
    
    // Simulate decoding process
    size_t decodedSize = std::min(inputBuffer.size(), outputBuffer.size());
    std::copy(inputBuffer.begin(), inputBuffer.begin() + decodedSize, outputBuffer.begin());
    
    return true; // Return true if decoding was successful
}

} //namespace sklepan
