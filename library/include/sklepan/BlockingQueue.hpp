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
    BlockingQueue(size_t size) {
        _pool.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            auto item = std::make_shared<T>();
            _pool.emplace_back(item);
            _free.push(item);
        }
    }

    /**
     * @brief Acquires a free item from the queue. Blocks if no free items are available.
     * 
     * @return A shared pointer to the acquired free item.
     */
    std::shared_ptr<T> acquireFree() {
        std::unique_lock lock(_mutex);
        _write_cv.wait(lock, [&] { return !_free.empty(); });

        auto item = _free.front();
        _free.pop();
        return item;
    }

    /**
     * @brief Marks an item as occupied and makes it available for reading.
     * 
     * @param item A shared pointer to the item to be marked as occupied.
     */
    void setOccupied(std::shared_ptr<T> item) {
        {
            std::scoped_lock lock(_mutex);
            _occupied.push(item);
        }
        _read_cv.notify_one();
    }

    /**
     * @brief Acquires the next occupied item from the queue. Blocks if no occupied items are available.
     * 
     * @return A shared pointer to the acquired occupied item.
     */
    std::shared_ptr<T> acquireOccupied() {
        std::unique_lock lock(_mutex);
        _read_cv.wait(lock, [&] { return !_occupied.empty(); });

        auto item = _occupied.front();
        _occupied.pop();
        return item;
    }

    /**
     * @brief Releases an item back to the free queue after it has been read.
     * 
     * @param item A shared pointer to the item to be released.
     */
    void release(std::shared_ptr<T> item) {
        {
            std::scoped_lock lock(_mutex);
            _free.push(item);
        }
        _write_cv.notify_one();
    }

private:
    std::vector<std::shared_ptr<T>> _pool;
    std::queue<std::shared_ptr<T>> _free;
    std::queue<std::shared_ptr<T>> _occupied;
    std::mutex _mutex;
    std::condition_variable _read_cv;
    std::condition_variable _write_cv;
};

} // namespace sklepan

