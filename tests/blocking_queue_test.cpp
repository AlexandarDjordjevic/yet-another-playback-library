#include "yapl/detail/blocking_queue.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>

using namespace yapl;

TEST(BlockingQueueTest, PushAndPopBasic) {
    blocking_queue<int> queue{10};

    queue.push(42);
    auto result = queue.try_pop();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(BlockingQueueTest, TryPopReturnsNulloptWhenEmpty) {
    blocking_queue<int> queue{10};

    auto result = queue.try_pop();

    EXPECT_FALSE(result.has_value());
}

TEST(BlockingQueueTest, IsEmptyAndSizeCorrect) {
    blocking_queue<int> queue{10};

    EXPECT_TRUE(queue.is_empty());
    EXPECT_EQ(queue.size(), 0u);

    queue.push(1);
    queue.push(2);

    EXPECT_FALSE(queue.is_empty());
    EXPECT_EQ(queue.size(), 2u);

    queue.try_pop();

    EXPECT_EQ(queue.size(), 1u);
}

TEST(BlockingQueueTest, ShutdownBlocksFurtherPushes) {
    blocking_queue<int> queue{10};

    queue.push(1);
    queue.shutdown();

    EXPECT_TRUE(queue.is_shutdown());

    // Further pushes should be ignored (check logs if needed)
    queue.push(2);

    // Only first item should be available
    auto result = queue.try_pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 1);

    result = queue.try_pop();
    EXPECT_FALSE(result.has_value());
}

TEST(BlockingQueueTest, StatsReportCorrectValues) {
    blocking_queue<int> queue{10};

    auto stats = queue.stats();
    EXPECT_EQ(stats.size, 0u);
    EXPECT_EQ(stats.capacity, 10u);

    queue.push(1);
    queue.push(2);

    stats = queue.stats();
    EXPECT_EQ(stats.size, 2u);
    EXPECT_EQ(stats.capacity, 10u);
}

TEST(BlockingQueueTest, FIFOOrdering) {
    blocking_queue<int> queue{10};

    for (int i = 1; i <= 5; ++i) {
        queue.push(i);
    }

    for (int i = 1; i <= 5; ++i) {
        auto result = queue.try_pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, i);
    }
}

TEST(BlockingQueueTest, ConcurrentPushAndPop) {
    blocking_queue<int> queue{100};
    constexpr int kNumItems = 1000;
    constexpr int kNumProducers = 4;
    constexpr int kNumConsumers = 4;
    constexpr int kItemsPerProducer = kNumItems / kNumProducers;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::vector<int> consumed_items;
    std::mutex consumed_mutex;

    // Start producers
    for (int p = 0; p < kNumProducers; ++p) {
        producers.emplace_back([&queue, p]() {
            for (int i = 0; i < kItemsPerProducer; ++i) {
                queue.push(p * kItemsPerProducer + i);
            }
        });
    }

    // Start consumers
    for (int c = 0; c < kNumConsumers; ++c) {
        consumers.emplace_back([&queue, &consumed_items, &consumed_mutex]() {
            while (true) {
                auto item = queue.try_pop();
                if (item.has_value()) {
                    std::lock_guard<std::mutex> lock(consumed_mutex);
                    consumed_items.push_back(*item);
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    if (queue.is_shutdown()) {
                        break;
                    }
                }
            }
        });
    }

    // Wait for producers to finish
    for (auto &t : producers) {
        t.join();
    }

    // Give consumers time to drain the queue
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    queue.shutdown();

    // Wait for consumers to finish
    for (auto &t : consumers) {
        t.join();
    }

    // Verify all items were consumed exactly once
    EXPECT_EQ(consumed_items.size(), static_cast<size_t>(kNumItems));

    std::sort(consumed_items.begin(), consumed_items.end());
    for (int i = 0; i < kNumItems; ++i) {
        EXPECT_EQ(consumed_items[i], i) << "Missing or duplicate item at index " << i;
    }
}

TEST(BlockingQueueTest, SharedPtrHandling) {
    blocking_queue<std::shared_ptr<int>> queue{10};

    auto ptr1 = std::make_shared<int>(42);
    auto ptr2 = std::make_shared<int>(100);

    queue.push(ptr1);
    queue.push(ptr2);

    auto result1 = queue.try_pop();
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(**result1, 42);

    auto result2 = queue.try_pop();
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(**result2, 100);
}
