#include "yapl/detail/data_sources/http.hpp"

#include <algorithm>
#include <chrono>
#include <curl/curl.h>
#include <stdexcept>

namespace yapl::data_sources {

http::http(std::string url) : m_url(std::move(url)) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

http::~http() {
    close();
    curl_global_cleanup();
}

void http::open() {
    if (m_is_open) {
        return;
    }

    m_curl_handle = curl_easy_init();
    if (!m_curl_handle) {
        throw std::runtime_error("Failed to initialize CURL handle");
    }

    // Reset state
    m_buffer.clear();
    m_read_position = 0;
    m_download_complete = false;
    m_stop_requested = false;
    m_download_error = false;
    m_error_message.clear();
    m_content_length = 0;

    // Configure CURL options
    curl_easy_setopt(m_curl_handle, CURLOPT_URL, m_url.c_str());
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_curl_handle, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(m_curl_handle, CURLOPT_USERAGENT, "yapl/1.0");
    curl_easy_setopt(m_curl_handle, CURLOPT_ACCEPT_ENCODING, "");  // Accept all encodings
    curl_easy_setopt(m_curl_handle, CURLOPT_BUFFERSIZE, 256 * 1024L);  // 256KB buffer

    // SSL options
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);

    // Start download in background thread
    m_download_thread = std::thread(&http::download_thread_func, this);

    // Wait for minimum buffer or completion
    {
        std::unique_lock lock(m_buffer_mutex);
        m_data_available_cv.wait(lock, [this] {
            return m_buffer.size() >= kMinBufferBeforeRead ||
                   m_download_complete ||
                   m_download_error;
        });
    }

    if (m_download_error) {
        if (m_download_thread.joinable()) {
            m_download_thread.join();
        }
        throw std::runtime_error("HTTP download failed: " + m_error_message);
    }

    m_is_open = true;
}

void http::close() {
    m_stop_requested = true;

    if (m_download_thread.joinable()) {
        m_download_thread.join();
    }

    if (m_curl_handle) {
        curl_easy_cleanup(m_curl_handle);
        m_curl_handle = nullptr;
    }

    m_is_open = false;
    m_buffer.clear();
    m_read_position = 0;
}

bool http::is_open() const {
    return m_is_open;
}

size_t http::read_data(size_t size, std::span<uint8_t> buffer) {
    if (!m_is_open) {
        throw std::runtime_error("HTTP source is not open: " + m_url);
    }

    if (buffer.size() < size) {
        throw std::invalid_argument("Buffer too small for requested read size");
    }

    if (size == 0) {
        return 0;
    }

    std::unique_lock lock(m_buffer_mutex);

    // Wait for data if not enough available and download still in progress
    m_data_available_cv.wait(lock, [this, size] {
        return (m_buffer.size() - m_read_position) >= size ||
               m_download_complete ||
               m_download_error;
    });

    if (m_download_error) {
        throw std::runtime_error("HTTP download error: " + m_error_message);
    }

    const size_t bytes_available = m_buffer.size() - m_read_position;
    if (bytes_available == 0) {
        return 0;  // EOF
    }

    const size_t bytes_to_read = std::min(size, bytes_available);
    std::copy_n(m_buffer.begin() + static_cast<std::ptrdiff_t>(m_read_position),
                bytes_to_read,
                buffer.begin());
    m_read_position += bytes_to_read;

    return bytes_to_read;
}

size_t http::available() const {
    std::lock_guard lock(m_buffer_mutex);
    return m_buffer.size() - m_read_position;
}

void http::reset() {
    std::lock_guard lock(m_buffer_mutex);
    m_read_position = 0;
}

size_t http::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* self = static_cast<http*>(userdata);
    const size_t total_size = size * nmemb;

    if (self->m_stop_requested) {
        return 0;  // Abort transfer
    }

    {
        std::lock_guard lock(self->m_buffer_mutex);
        self->m_buffer.insert(self->m_buffer.end(),
                              reinterpret_cast<uint8_t*>(ptr),
                              reinterpret_cast<uint8_t*>(ptr) + total_size);
    }

    self->m_data_available_cv.notify_one();
    return total_size;
}

size_t http::header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* self = static_cast<http*>(userdata);
    const size_t total_size = size * nitems;
    const std::string header(buffer, total_size);

    // Parse Content-Length header
    constexpr std::string_view content_length_prefix = "Content-Length:";
    if (header.starts_with(content_length_prefix)) {
        const std::string value = header.substr(content_length_prefix.size());
        try {
            self->m_content_length = std::stoull(value);
        } catch (...) {
            // Ignore parse errors
        }
    }

    return total_size;
}

void http::download_thread_func() {
    const CURLcode result = curl_easy_perform(m_curl_handle);

    if (result != CURLE_OK && result != CURLE_ABORTED_BY_CALLBACK) {
        m_download_error = true;
        m_error_message = curl_easy_strerror(result);
    }

    // Check HTTP response code
    if (!m_download_error) {
        long http_code = 0;
        curl_easy_getinfo(m_curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            m_download_error = true;
            m_error_message = "HTTP error: " + std::to_string(http_code);
        }
    }

    m_download_complete = true;
    m_data_available_cv.notify_all();
}

}  // namespace yapl::data_sources
