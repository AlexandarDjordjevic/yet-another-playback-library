/**
 * @file blocking_queue_bench.cpp
 * @brief Performance benchmarks for blocking_queue
 *
 * Measures throughput and latency of queue operations under various conditions.
 */

#include "yapl/detail/blocking_queue.hpp"
#include <benchmark/benchmark.h>
#include <memory>
#include <thread>
#include <vector>

using namespace yapl;

/**
 * @brief Benchmark single-threaded push operations
 */
static void BM_BlockingQueue_Push(benchmark::State &state) {
    const size_t queue_size = state.range(0);
    blocking_queue<int> queue{queue_size};

    int value = 0;
    for (auto _ : state) {
        queue.push(value++);
        if (value % queue_size == 0) {
            // Drain queue to prevent blocking
            while (queue.try_pop()) {
            }
            value = 0;
        }
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BlockingQueue_Push)->Arg(10)->Arg(60)->Arg(100)->Arg(1024);

/**
 * @brief Benchmark single-threaded pop operations
 */
static void BM_BlockingQueue_Pop(benchmark::State &state) {
    const size_t queue_size = state.range(0);
    blocking_queue<int> queue{queue_size};

    // Pre-fill queue
    for (size_t i = 0; i < queue_size; ++i) {
        queue.push(static_cast<int>(i));
    }

    size_t refill_counter = 0;
    for (auto _ : state) {
        auto result = queue.try_pop();
        benchmark::DoNotOptimize(result);

        if (++refill_counter % queue_size == 0) {
            // Refill queue
            for (size_t i = 0; i < queue_size; ++i) {
                queue.push(static_cast<int>(i));
            }
            refill_counter = 0;
        }
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BlockingQueue_Pop)->Arg(10)->Arg(60)->Arg(100)->Arg(1024);

/**
 * @brief Benchmark push/pop with shared_ptr (realistic media frame scenario)
 */
static void BM_BlockingQueue_SharedPtr(benchmark::State &state) {
    const size_t queue_size = state.range(0);
    const size_t frame_size = state.range(1);
    blocking_queue<std::shared_ptr<std::vector<uint8_t>>> queue{queue_size};

    size_t counter = 0;
    for (auto _ : state) {
        // Simulate frame allocation
        auto frame = std::make_shared<std::vector<uint8_t>>(frame_size);
        queue.push(frame);

        if (++counter % queue_size == 0) {
            // Drain queue
            while (queue.try_pop()) {
            }
            counter = 0;
        }
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * frame_size);
}
BENCHMARK(BM_BlockingQueue_SharedPtr)
    ->Args({60, 1920 * 1080 * 3 / 2})  // 1080p YUV420
    ->Args({60, 1280 * 720 * 3 / 2});  // 720p YUV420

/**
 * @brief Benchmark concurrent push/pop (single producer, single consumer)
 */
static void BM_BlockingQueue_SPSC(benchmark::State &state) {
    const size_t queue_size = state.range(0);
    blocking_queue<int> queue{queue_size};

    std::atomic<bool> done{false};
    std::atomic<size_t> consumed{0};

    // Consumer thread
    std::thread consumer([&]() {
        while (!done.load(std::memory_order_relaxed)) {
            if (queue.try_pop()) {
                consumed.fetch_add(1, std::memory_order_relaxed);
            }
        }
        // Drain remaining items
        while (queue.try_pop()) {
            consumed.fetch_add(1, std::memory_order_relaxed);
        }
    });

    // Producer (benchmark thread)
    size_t produced = 0;
    for (auto _ : state) {
        queue.push(static_cast<int>(produced++));
    }

    done.store(true, std::memory_order_relaxed);
    consumer.join();

    state.SetItemsProcessed(state.iterations());
    state.counters["consumed"] = consumed.load();
}
BENCHMARK(BM_BlockingQueue_SPSC)->Arg(60)->Arg(1024);

/**
 * @brief Benchmark concurrent push/pop (multiple producers, multiple consumers)
 */
static void BM_BlockingQueue_MPMC(benchmark::State &state) {
    const size_t queue_size = state.range(0);
    const size_t num_consumers = state.range(1);
    blocking_queue<int> queue{queue_size};

    std::atomic<bool> done{false};
    std::atomic<size_t> consumed{0};

    // Consumer threads
    std::vector<std::thread> consumers;
    for (size_t i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&]() {
            while (!done.load(std::memory_order_acquire)) {
                if (queue.try_pop()) {
                    consumed.fetch_add(1, std::memory_order_relaxed);
                }
            }
            // Drain remaining
            while (queue.try_pop()) {
                consumed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Producers (benchmark threads)
    size_t produced = 0;
    for (auto _ : state) {
        queue.push(static_cast<int>(produced++));
    }

    done.store(true, std::memory_order_release);
    for (auto &t : consumers) {
        t.join();
    }

    state.SetItemsProcessed(state.iterations());
    state.counters["consumed"] = consumed.load();
    state.counters["consumers"] = num_consumers;
}
BENCHMARK(BM_BlockingQueue_MPMC)
    ->Args({1024, 1})
    ->Args({1024, 2})
    ->Args({1024, 4});

/**
 * @brief Benchmark queue stats query performance
 */
static void BM_BlockingQueue_Stats(benchmark::State &state) {
    const size_t queue_size = state.range(0);
    blocking_queue<int> queue{queue_size};

    // Fill queue partially
    for (size_t i = 0; i < queue_size / 2; ++i) {
        queue.push(static_cast<int>(i));
    }

    for (auto _ : state) {
        auto stats = queue.stats();
        benchmark::DoNotOptimize(stats);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BlockingQueue_Stats)->Arg(60)->Arg(1024);
