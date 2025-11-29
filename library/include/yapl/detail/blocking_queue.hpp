#pragma once

#include "yapl/pipeline_stats.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>

namespace yapl {

template <typename T> class blocking_queue {
  public:
    enum class pop_result { no_error, timeout, shutdown };

    struct pop_output {
        pop_result result;
        std::optional<T> data;
    };

    explicit blocking_queue(size_t capacity)
        : m_capacity(capacity), m_head(0), m_tail(0), m_count(0), m_shutdown(false) {
        if (capacity == 0)
            throw std::invalid_argument("blocking_queue requires capacity >= 1");
        m_pool.resize(capacity);
    }

    // Shutdown the queue - wakes up all blocked threads
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_shutdown = true;
        }
        m_not_empty.notify_all();
        m_not_full.notify_all();
    }

    // Returns false if shutdown
    bool push(const T &item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_not_full.wait(lock, [this] { return m_count < m_capacity || m_shutdown; });

        if (m_shutdown) return false;

        m_pool[m_tail] = item;
        m_tail = (m_tail + 1) % m_capacity;
        ++m_count;

        m_not_empty.notify_one();
        return true;
    }

    // Returns nullopt if shutdown or empty
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_not_empty.wait(lock, [this] { return m_count > 0 || m_shutdown; });

        if (m_shutdown && m_count == 0) return std::nullopt;

        T item = std::move(m_pool[m_head]);
        m_head = (m_head + 1) % m_capacity;
        --m_count;

        m_not_full.notify_one();
        return item;
    }

    pop_output pop(std::chrono::milliseconds timeout_ms) {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool success = m_not_empty.wait_for(lock, timeout_ms,
                                            [this] { return m_count > 0 || m_shutdown; });
        if (m_shutdown && m_count == 0) {
            return {.result = pop_result::shutdown, .data = std::nullopt};
        }
        if (!success) {
            return {.result = pop_result::timeout, .data = std::nullopt};
        }

        T item = std::move(m_pool[m_head]);
        m_head = (m_head + 1) % m_capacity;
        --m_count;

        m_not_full.notify_one();
        return {.result = pop_result::no_error, .data = std::make_optional(std::move(item))};
    }

    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_count == 0) return std::nullopt;
        T item = std::move(m_pool[m_head]);
        m_head = (m_head + 1) % m_capacity;
        --m_count;
        m_not_full.notify_one();
        return item;
    }

    bool try_push(const T &item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_count == m_capacity || m_shutdown) return false;
        m_pool[m_tail] = item;
        m_tail = (m_tail + 1) % m_capacity;
        ++m_count;
        m_not_empty.notify_one();
        return true;
    }

    [[nodiscard]] size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count;
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
        return m_capacity;
    }

    [[nodiscard]] queue_stats stats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return {.size = m_count, .capacity = m_capacity};
    }

    bool is_empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count == 0;
    }

    bool is_full() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count == m_capacity;
    }

    bool is_shutdown() const {
        return m_shutdown;
    }

  private:
    std::vector<T> m_pool;
    const size_t m_capacity;
    size_t m_head;
    size_t m_tail;
    size_t m_count;
    std::atomic_bool m_shutdown;

    mutable std::mutex m_mutex;
    std::condition_variable m_not_empty;
    std::condition_variable m_not_full;
};

} // namespace yapl
