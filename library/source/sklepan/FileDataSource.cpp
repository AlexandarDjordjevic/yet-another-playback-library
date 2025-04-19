#include "sklepan/FileDataSource.hpp"
#include "sklepan/debug.hpp"

#include <fstream>

namespace sklepan {

FileDataSource::FileDataSource(const std::string_view filePath) : _filePath(filePath) {}

void FileDataSource::open() {
    LOG_INFO("Opening file data source: {}", std::string(_filePath));
    _file.open(_filePath, std::ios::in | std::ios::binary);
    if (!_file.is_open()) {
        throw std::runtime_error("Could not open file: " + _filePath);
    }
    _file.seekg(0, std::ios::end);
    _fileSize = _file.tellg();
    _file.seekg(0, std::ios::beg);
    _currentPosition = 0;
    LOG_INFO("File size: {} bytes", _fileSize);
}

void FileDataSource::close() {
    _file.close();
}

bool FileDataSource::isOpen() const {
    return _file.is_open();
}

size_t FileDataSource::readData(size_t size, std::span<uint8_t> buffer) {
    LOG_DEBUG("Reading {} bytes from file data source", size);
    if (!_file.is_open()) {
        throw std::runtime_error("File is not open: " + _filePath);
    }
    _file.read(reinterpret_cast<char*>(buffer.data()), size);
    return size;
}

size_t FileDataSource::available() const {
    return _fileSize - _currentPosition;
}

} // namespace sklepan
