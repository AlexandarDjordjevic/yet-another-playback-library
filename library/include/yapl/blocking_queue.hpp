#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>

namespace yapl {

template <typename T> class data_queue {
  public:
    enum class pop_result { no_error, timeout };

    struct pop_output {
        pop_result result;
        std::optional<T> data;
    };

    explicit data_queue(size_t capacity)
        : m_capacity(capacity), m_head(0), m_tail(0), m_count(0) {
        if (capacity == 0)
            throw std::invalid_argument("data_queue requires capacity >= 1");
        m_pool.resize(capacity);
    }

    void push(const T &item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_not_full.wait(lock, [this] { return m_count < m_capacity; });

        m_pool[m_tail] = item;
        m_tail = (m_tail + 1) % m_capacity;
        ++m_count;

        m_not_empty.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_not_empty.wait(lock, [this] { return m_count > 0; });

        T item = std::move(m_pool[m_head]);
        m_head = (m_head + 1) % m_capacity;
        --m_count;

        m_not_full.notify_one();
        return item;
    }

    pop_output pop(std::chrono::milliseconds timeout_ms) {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool success = m_not_empty.wait_for(lock, timeout_ms,
                                            [this] { return m_count > 0; });
        if (!success) {
            return {.result = pop_result::timeout, .data = std::nullopt};
        }

        T item = std::move(m_pool[m_head]);
        m_head = (m_head + 1) % m_capacity;
        --m_count;

        m_not_full.notify_one();
        return {.result = pop_result::no_error,
                .data = std::make_optional(std::move(item))};
    }

    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_count == 0)
            return std::nullopt;
        T item = std::move(m_pool[m_head]);
        m_head = (m_head + 1) % m_capacity;
        --m_count;
        m_not_full.notify_one();
        return item;
    }

    bool try_push(const T &item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_count == m_capacity)
            return false;
        m_pool[m_tail] = item;
        m_tail = (m_tail + 1) % m_capacity;
        ++m_count;
        m_not_empty.notify_one();
        return true;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count;
    }

    size_t capacity() const noexcept { return m_capacity; }

    bool is_empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count == 0;
    }

    bool is_full() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count == m_capacity;
    }

  private:
    std::vector<T> m_pool;
    const size_t m_capacity;
    size_t m_head;
    size_t m_tail;
    size_t m_count;

    std::mutex m_mutex;
    std::condition_variable m_not_empty;
    std::condition_variable m_not_full;
};

} // namespace yapl
