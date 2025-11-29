#include "yapl/detail/media_source.hpp"

#include <stdexcept>

namespace yapl {

media_source::media_source() : m_data_source{std::unique_ptr<data_sources::file>{}} {}

void media_source::open(const std::string_view url) {
    m_data_source = data_sources::create(url);
    data_sources::visit(m_data_source, [](auto& ds) { ds.open(); });
}

void media_source::close() {
    data_sources::visit(m_data_source, [](auto& ds) { ds.close(); });
}

size_t media_source::read_packet(size_t size, std::span<uint8_t> buffer) {
    auto is_open = data_sources::visit(m_data_source, [](const auto& ds) { return ds.is_open(); });
    if (!is_open) {
        throw std::runtime_error("Media source is not open");
    }
    return data_sources::visit(m_data_source, [size, buffer](auto& ds) {
        return ds.read_data(size, buffer);
    });
}

size_t media_source::available() const {
    auto is_open = data_sources::visit(m_data_source, [](const auto& ds) { return ds.is_open(); });
    if (!is_open) {
        throw std::runtime_error("Media source is not open");
    }
    return data_sources::visit(m_data_source, [](const auto& ds) { return ds.available(); });
}

void media_source::reset() {
    data_sources::visit(m_data_source, [](auto& ds) { ds.reset(); });
}

} // namespace yapl
