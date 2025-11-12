#pragma once

#include "yapl/idata_source.hpp"

#include <fstream>
#include <string>

namespace yapl::data_sources {

class file_data_source : public idata_source {
  public:
    file_data_source(const std::string_view filePath);
    ~file_data_source() = default;
    file_data_source(const file_data_source &) = delete;
    file_data_source &operator=(const file_data_source &) = delete;

    void open() override;
    void close() override;
    bool is_open() const override;
    size_t read_data(size_t size, std::span<uint8_t> buffer) override;
    size_t available() const override;
    void reset() override;

  private:
    std::string m_file_path;
    std::ifstream m_file;
    size_t m_file_size = 0;
    size_t m_current_position = 0;
};

} // namespace yapl::data_sources
