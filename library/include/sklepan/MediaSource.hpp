#pragma once

#include "BlockingQueue.hpp"
#include "IDataSource.hpp"
#include "IMediaSource.hpp"
#include "IMediaExtractor.hpp"

#include <memory>
#include <thread>

namespace sklepan
{

    #define RAW_BUFFER_SIZE 2 * 1024 * 1024 // 2MB

struct RawDataBuffer{
    RawDataBuffer() : size{0}, capacity{RAW_BUFFER_SIZE}, data{std::make_shared<uint8_t[]>(RAW_BUFFER_SIZE)} {}
    size_t size;
    size_t capacity;
    std::shared_ptr<uint8_t[]> data;
};

struct MediaSource : public IMediaSource{
    MediaSource();

    void open(const std::string_view url) override;

    void close() override;

    size_t readPacket(size_t size, std::span<uint8_t> buffer) override;

    size_t available() const override;

private:
    std::shared_ptr<IDataSource> _dataSource;
};

}// namespace sklepan
