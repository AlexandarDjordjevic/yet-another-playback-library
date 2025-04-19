#pragma once

#include <span>
#include <iterator>
#include <queue>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <mutex>
#include <condition_variable>

namespace sklepan{

template <typename T>
class BlockingQueue {
public:
    /**
     * @brief Constructs a BlockingQueue with the specified size.
     * 
     * @param size The number of items the queue can hold.
     */
    BlockingQueue(size_t size) : _size(size), _head(0), _tail(0) {
        _pool.resize(size);
    }

    void push(const T& item) {
        std::unique_lock<std::mutex> lock(_mutex);
        _write_cv.wait(lock, [this] { return (_head + 1) % _size != _tail; });
        _pool[_head] = item;
        _head = (_head + 1) % _size;
        lock.unlock();
        _read_cv.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(_mutex);
        _read_cv.wait(lock, [this] { return _head != _tail; });
        T item = _pool[_tail];
        _tail = (_tail + 1) % _size;
        lock.unlock();
        _write_cv.notify_one();
        return item;
    }
private:
    std::vector<T> _pool;
    size_t _size;
    size_t _head;
    size_t _tail;
    std::mutex _mutex;
    std::condition_variable _read_cv;
    std::condition_variable _write_cv;
};

} // namespace sklepan

