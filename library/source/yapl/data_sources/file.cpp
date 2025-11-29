#include "yapl/detail/data_sources/file.hpp"

#include <algorithm>
#include <stdexcept>

namespace yapl::data_sources {

file::file(std::filesystem::path file_path)
    : m_file_path(std::move(file_path)) {}

file::~file() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

void file::open() {
    m_file.open(m_file_path, std::ios::in | std::ios::binary);
    if (!m_file.is_open()) {
        throw std::runtime_error("Could not open file: " +
                                 m_file_path.string());
    }

    m_file.seekg(0, std::ios::end);
    if (!m_file) {
        m_file.close();
        throw std::runtime_error("Failed to seek in file: " +
                                 m_file_path.string());
    }

    m_file_size = static_cast<size_t>(m_file.tellg());
    m_file.seekg(0, std::ios::beg);
    m_current_position = 0;
}

void file::close() { m_file.close(); }

bool file::is_open() const { return m_file.is_open(); }

size_t file::read_data(size_t size, std::span<uint8_t> buffer) {
    if (!m_file.is_open()) {
        throw std::runtime_error("File is not open: " + m_file_path.string());
    }

    if (buffer.size() < size) {
        throw std::invalid_argument("Buffer too small for requested read size");
    }

    if (size == 0 || m_current_position >= m_file_size) {
        return 0;
    }

    const size_t bytes_to_read =
        std::min(size, m_file_size - m_current_position);
    m_file.read(reinterpret_cast<char *>(buffer.data()),
                static_cast<std::streamsize>(bytes_to_read));
    const auto bytes_read = static_cast<size_t>(m_file.gcount());
    m_current_position += bytes_read;

    return bytes_read;
}

size_t file::available() const { return m_file_size - m_current_position; }

void file::reset() {
    m_current_position = 0;
    m_file.seekg(0, std::ios::beg);
    m_file.clear(); // Clear any EOF or error flags
}

} // namespace yapl::data_sources
