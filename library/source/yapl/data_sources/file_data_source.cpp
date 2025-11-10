#include "yapl/data_sources/file_data_source.hpp"
#include "yapl/debug.hpp"

namespace yapl::data_sources {

file_data_source::file_data_source(const std::string_view file_path)
    : m_file_path(file_path) {}

void file_data_source::open() {
    LOG_DEBUG("File data source -> open file {}", std::string(m_file_path));
    m_file.open(m_file_path, std::ios::in | std::ios::binary);
    if (!m_file.is_open()) {
        throw std::runtime_error("Could not open file: " + m_file_path);
    }
    m_file.seekg(0, std::ios::end);
    m_file_size = m_file.tellg();
    m_file.seekg(0, std::ios::beg);
    m_current_position = 0;
    LOG_DEBUG("File data source -> file size: {} bytes", m_file_size);
}

void file_data_source::close() { m_file.close(); }

bool file_data_source::is_open() const { return m_file.is_open(); }

size_t file_data_source::read_data(size_t size, std::span<uint8_t> buffer) {
    LOG_DEBUG("Reading {} bytes from file data source", size);
    if (!m_file.is_open()) {
        throw std::runtime_error("File is not open: " + m_file_path);
    }
    m_file.read(reinterpret_cast<char *>(buffer.data()), size);
    return size;
}

size_t file_data_source::available() const {
    return m_file_size - m_current_position;
}

} // namespace yapl::data_sources
