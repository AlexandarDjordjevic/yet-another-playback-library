#pragma once

#include "yapl/i_data_source.hpp"

#include <filesystem>
#include <fstream>

namespace yapl::data_sources {

class file {
  public:
    explicit file(std::filesystem::path file_path);
    ~file();

    file(const file &) = delete;
    file &operator=(const file &) = delete;
    file(file &&) = default;
    file &operator=(file &&) = default;

    void open();
    void close();
    [[nodiscard]] bool is_open() const;
    size_t read_data(size_t size, std::span<uint8_t> buffer);
    [[nodiscard]] size_t available() const;
    void reset();

  private:
    std::filesystem::path m_file_path;
    std::ifstream m_file;
    size_t m_file_size = 0;
    size_t m_current_position = 0;
};

static_assert(data_source<file>, "file must satisfy data_source concept");

} // namespace yapl::data_sources
