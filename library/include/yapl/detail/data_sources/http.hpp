#pragma once

#include "yapl/i_data_source.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

typedef void CURL;

namespace yapl::data_sources {

class http {
  public:
    explicit http(std::string url);
    ~http();

    http(const http &) = delete;
    http &operator=(const http &) = delete;
    http(http &&) = delete;
    http &operator=(http &&) = delete;

    void open();
    void close();
    [[nodiscard]] bool is_open() const;
    size_t read_data(size_t size, std::span<uint8_t> buffer);
    [[nodiscard]] size_t available() const;
    void reset();

    [[nodiscard]] std::string get_url() const { return m_url; }
    [[nodiscard]] size_t get_content_length() const { return m_content_length; }

  private:
    static size_t write_callback(char *ptr, size_t size, size_t nmemb,
                                 void *userdata);
    static size_t header_callback(char *buffer, size_t size, size_t nitems,
                                  void *userdata);
    void download_thread_func();

    std::string m_url;
    CURL *m_curl_handle{nullptr};
    bool m_is_open{false};

    // Internal buffer for downloaded data
    std::vector<uint8_t> m_buffer;
    size_t m_read_position{0};
    size_t m_content_length{0};
    mutable std::mutex m_buffer_mutex;
    std::condition_variable m_data_available_cv;

    // Download thread management
    std::thread m_download_thread;
    std::atomic_bool m_download_complete{false};
    std::atomic_bool m_stop_requested{false};
    std::atomic_bool m_download_error{false};
    std::string m_error_message;

    static constexpr size_t kMinBufferBeforeRead =
        512 * 1024; // 512KB minimum before allowing reads
};

// Static assertion to verify http satisfies data_source concept
static_assert(data_source<http>, "http must satisfy data_source concept");

} // namespace yapl::data_sources
