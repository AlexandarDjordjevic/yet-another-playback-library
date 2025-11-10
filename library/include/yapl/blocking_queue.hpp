#pragma once

#include <condition_variable>
#include <cstring>
#include <mutex>
#include <vector>

namespace yapl {

template <typename T> class blocking_queue {
  public:
    /**
     * @brief Constructs a blocking_queue with the specified size.
     *
     * @param size The number of items the queue can hold.
     */
    blocking_queue(size_t size) : m_size(size), m_head(0), m_tail(0) {
        m_pool.resize(size);
    }

    void push(const T &item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_write_cv.wait(lock,
                        [this] { return (m_head + 1) % m_size != m_tail; });
        m_pool[m_head] = item;
        m_head = (m_head + 1) % m_size;
        lock.unlock();
        m_read_cv.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_read_cv.wait(lock, [this] { return m_head != m_tail; });
        T item = m_pool[m_tail];
        m_tail = (m_tail + 1) % m_size;
        lock.unlock();
        m_write_cv.notify_one();
        return item;
    }

  private:
    std::vector<T> m_pool;
    size_t m_size;
    size_t m_head;
    size_t m_tail;
    std::mutex m_mutex;
    std::condition_variable m_read_cv;
    std::condition_variable m_write_cv;
};

} // namespace yapl
