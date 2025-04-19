#pragma once

#include "IDataSource.hpp"

#include <string>
#include <fstream>

namespace sklepan {

class FileDataSource : public IDataSource {
public:
    FileDataSource(const std::string_view filePath);
    ~FileDataSource() = default;
    FileDataSource(const FileDataSource&) = delete;
    FileDataSource& operator=(const FileDataSource&) = delete;
    
    void open() override;
    void close() override;
    bool isOpen() const override;
    size_t readData(size_t size, std::span<uint8_t> buffer) override;
    size_t available() const override;

private:
    std::string _filePath;
    std::ifstream _file;
    size_t _fileSize = 0;
    size_t _currentPosition = 0;
};

} // namespace sklepan
