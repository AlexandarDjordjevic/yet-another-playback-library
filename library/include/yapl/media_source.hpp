#pragma once

// #include "BlockingQueue.hpp"
#include "yapl/idata_source.hpp"
#include "yapl/imedia_source.hpp"

#include <memory>

namespace yapl {

#define RAW_BUFFER_SIZE 2 * 1024 * 1024 // 2MB

struct raw_data_buffer {
    raw_data_buffer()
        : size{0}, capacity{RAW_BUFFER_SIZE},
          data{std::make_shared<uint8_t[]>(RAW_BUFFER_SIZE)} {}
    size_t size;
    size_t capacity;
    std::shared_ptr<uint8_t[]> data;
};

struct media_source : public imedia_source {
    media_source();

    void open(const std::string_view url) override;

    void close() override;

    size_t read_packet(size_t size, std::span<uint8_t> buffer) override;

    size_t available() const override;

    void reset() override;

  private:
    std::shared_ptr<idata_source> m_data_source;
};

} // namespace yapl
