#pragma once

#include "yapl/detail/data_sources/factory.hpp"
#include "yapl/i_media_source.hpp"

#include <memory>

namespace yapl {

// Forces compile-time evaluation of buffer size calculation
consteval size_t compute_buffer_size_mb(size_t mb) noexcept {
    return mb * 1024 * 1024;
}

inline constexpr size_t kRawBufferSize = compute_buffer_size_mb(2); // 2MB

struct raw_data_buffer {
    raw_data_buffer()
        : size{0}, capacity{kRawBufferSize},
          data{std::make_shared<uint8_t[]>(kRawBufferSize)} {}
    size_t size;
    size_t capacity;
    std::shared_ptr<uint8_t[]> data;
};

struct media_source : public i_media_source {
    media_source();

    void open(const std::string_view url) override;

    void close() override;

    size_t read_packet(size_t size, std::span<uint8_t> buffer) override;

    size_t available() const override;

    void reset() override;

  private:
    data_sources::data_source_variant m_data_source;
};

} // namespace yapl
