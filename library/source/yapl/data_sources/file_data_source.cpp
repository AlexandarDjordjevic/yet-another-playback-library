#include "yapl/data_sources/file_data_source.hpp"
#include "yapl/debug.hpp"

namespace yapl::data_sources {

file_data_source::file_data_source(const std::string_view file_path)
    : m_file_path(file_path) {}

void file_data_source::open() {
    m_file.open(m_file_path, std::ios::in | std::ios::binary);
    if (!m_file.is_open()) {
        throw std::runtime_error("Could not open file: " + m_file_path);
    }
    m_file.seekg(0, std::ios::end);
    m_file_size = m_file.tellg();
    m_file.seekg(0, std::ios::beg);
    m_current_position = 0;
}

void file_data_source::close() { m_file.close(); }

bool file_data_source::is_open() const { return m_file.is_open(); }

// bool file_data_source::seek_position(int64_t position) const {
//     if (!m_file.is_open()) {
//         throw std::runtime_error("File is not open: " + m_file_path);
//     }

//     if (position < 0) {
//          m_file.seekg(0, std::ios::end);
//     }

//         if (position < 0) {
//          m_file.seekg(0, std::ios::end);
//     }

// }

size_t file_data_source::read_data(size_t size, std::span<uint8_t> buffer) {
    if (!m_file.is_open()) {
        throw std::runtime_error("File is not open: " + m_file_path);
    }
    if (!size) {
        return size;
    }

    m_file.read(reinterpret_cast<char *>(buffer.data()), size);
    auto read = m_file.gcount();

    if (m_current_position >= m_file_size) {
        LOG_INFO("EOF Reached");
        return 0;
    } else {
        m_current_position += read;
    }

    return read;
}

size_t file_data_source::available() const {
    return m_file_size - m_current_position;
}

void file_data_source::reset() {
    // m_current_position = 0;
    // m_file.seekg(0, std::ios::beg);
}

} // namespace yapl::data_sources
