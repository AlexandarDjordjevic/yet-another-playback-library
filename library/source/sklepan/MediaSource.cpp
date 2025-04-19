#include "sklepan/MediaSource.hpp"
#include "sklepan/FileDataSource.hpp"
#include "sklepan/FFMpegMediaExtractor.hpp"
#include "sklepan/debug.hpp"

#include <fstream>

using namespace std::chrono_literals;

namespace sklepan {

MediaSource::MediaSource() : _dataSource{std::make_shared<FileDataSource>("BigBuckBunny.mp4")} {
    LOG_INFO("MediaSource constructor called");
}

void MediaSource::open(const std::string_view url) {
    LOG_INFO("Opening media source");
    _dataSource = std::make_shared<FileDataSource>(url);
    _dataSource->open();
}

void MediaSource::close() {
    _dataSource->close();
}

size_t MediaSource::readPacket(size_t size, std::span<uint8_t> buffer) {
    LOG_DEBUG("Reading {} bytes from media source", size);
    if(_dataSource->isOpen() == false) {
        throw std::runtime_error("Media source is not open");
    }
    return _dataSource->readData(size, buffer);
}

size_t MediaSource::available() const {
    if(_dataSource->isOpen() == false) {
        throw std::runtime_error("Media source is not open");
    }
    return _dataSource->available();
}

} // namespace sklepan
