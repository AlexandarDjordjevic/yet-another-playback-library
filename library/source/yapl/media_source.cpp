#include "yapl/media_source.hpp"
#include "yapl/data_sources/file_data_source.hpp"
#include "yapl/debug.hpp"

using namespace std::chrono_literals;

namespace yapl {

media_source::media_source()
    : m_data_source{std::make_shared<data_sources::file_data_source>(
          "BigBuckBunny.mp4")} {}

void media_source::open(const std::string_view url) {
    m_data_source = std::make_shared<data_sources::file_data_source>(url);
    m_data_source->open();
}

void media_source::close() { m_data_source->close(); }

size_t media_source::read_packet(size_t size, std::span<uint8_t> buffer) {
    if (m_data_source->is_open() == false) {
        throw std::runtime_error("Media source is not open");
    }
    return m_data_source->read_data(size, buffer);
}

size_t media_source::available() const {
    if (m_data_source->is_open() == false) {
        throw std::runtime_error("Media source is not open");
    }
    return m_data_source->available();
}

void media_source::reset() { m_data_source->reset(); }

} // namespace yapl
