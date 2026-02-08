/**
 * @file memory_allocation_bench.cpp
 * @brief Memory allocation and frame handling benchmarks
 *
 * Measures allocation patterns and memory usage for media frames.
 */

#include "yapl/media_sample.hpp"
#include <benchmark/benchmark.h>
#include <memory>
#include <vector>

using namespace yapl;

/**
 * @brief Benchmark frame allocation (1080p YUV420)
 */
static void BM_FrameAllocation_1080p(benchmark::State &state) {
    constexpr size_t width = 1920;
    constexpr size_t height = 1080;
    constexpr size_t frame_size = width * height * 3 / 2; // YUV420

    for (auto _ : state) {
        auto frame = std::make_shared<media_sample>();
        frame->data.resize(frame_size);
        benchmark::DoNotOptimize(frame);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * frame_size);
}
BENCHMARK(BM_FrameAllocation_1080p);

/**
 * @brief Benchmark frame allocation (720p YUV420)
 */
static void BM_FrameAllocation_720p(benchmark::State &state) {
    constexpr size_t width = 1280;
    constexpr size_t height = 720;
    constexpr size_t frame_size = width * height * 3 / 2;

    for (auto _ : state) {
        auto frame = std::make_shared<media_sample>();
        frame->data.resize(frame_size);
        benchmark::DoNotOptimize(frame);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * frame_size);
}
BENCHMARK(BM_FrameAllocation_720p);

/**
 * @brief Benchmark frame allocation (4K YUV420)
 */
static void BM_FrameAllocation_4K(benchmark::State &state) {
    constexpr size_t width = 3840;
    constexpr size_t height = 2160;
    constexpr size_t frame_size = width * height * 3 / 2;

    for (auto _ : state) {
        auto frame = std::make_shared<media_sample>();
        frame->data.resize(frame_size);
        benchmark::DoNotOptimize(frame);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * frame_size);
}
BENCHMARK(BM_FrameAllocation_4K);

/**
 * @brief Benchmark audio frame allocation (44.1kHz stereo, 1024 samples)
 */
static void BM_AudioFrameAllocation(benchmark::State &state) {
    constexpr size_t samples = 1024;
    constexpr size_t channels = 2;
    constexpr size_t bytes_per_sample = sizeof(float);
    constexpr size_t frame_size = samples * channels * bytes_per_sample;

    for (auto _ : state) {
        auto frame = std::make_shared<media_sample>();
        frame->data.resize(frame_size);
        benchmark::DoNotOptimize(frame);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * frame_size);
}
BENCHMARK(BM_AudioFrameAllocation);

/**
 * @brief Benchmark frame copy operations
 */
static void BM_FrameCopy_1080p(benchmark::State &state) {
    constexpr size_t width = 1920;
    constexpr size_t height = 1080;
    constexpr size_t frame_size = width * height * 3 / 2;

    auto src = std::make_shared<media_sample>();
    src->data.resize(frame_size);

    for (auto _ : state) {
        auto dst = std::make_shared<media_sample>();
        dst->data = src->data; // Vector copy
        benchmark::DoNotOptimize(dst);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * frame_size);
}
BENCHMARK(BM_FrameCopy_1080p);

/**
 * @brief Benchmark shared_ptr reference counting overhead
 */
static void BM_SharedPtrRefCount(benchmark::State &state) {
    auto frame = std::make_shared<media_sample>();
    frame->data.resize(1920 * 1080 * 3 / 2);

    for (auto _ : state) {
        // Simulate passing frame through pipeline
        auto copy1 = frame;
        auto copy2 = copy1;
        auto copy3 = copy2;
        benchmark::DoNotOptimize(copy3);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_SharedPtrRefCount);

/**
 * @brief Benchmark memory pool vs individual allocations
 */
static void BM_FramePool_vs_Individual(benchmark::State &state) {
    const size_t pool_size = state.range(0);
    constexpr size_t frame_size = 1920 * 1080 * 3 / 2;

    // Pre-allocate pool
    std::vector<std::shared_ptr<media_sample>> pool;
    pool.reserve(pool_size);
    for (size_t i = 0; i < pool_size; ++i) {
        auto frame = std::make_shared<media_sample>();
        frame->data.resize(frame_size);
        pool.push_back(frame);
    }

    size_t index = 0;
    for (auto _ : state) {
        // Reuse from pool
        auto frame = pool[index++ % pool_size];
        benchmark::DoNotOptimize(frame);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * frame_size);
}
BENCHMARK(BM_FramePool_vs_Individual)->Arg(10)->Arg(60)->Arg(120);

/**
 * @brief Benchmark vector reserve vs resize
 */
static void BM_VectorReserve_vs_Resize(benchmark::State &state) {
    constexpr size_t frame_size = 1920 * 1080 * 3 / 2;
    const bool use_reserve = state.range(0);

    for (auto _ : state) {
        std::vector<uint8_t> data;
        if (use_reserve) {
            data.reserve(frame_size);
            data.insert(data.end(), frame_size, 0);
        } else {
            data.resize(frame_size);
        }
        benchmark::DoNotOptimize(data);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * frame_size);
}
BENCHMARK(BM_VectorReserve_vs_Resize)->Arg(0)->Arg(1);

/**
 * @brief Benchmark media_sample struct overhead
 */
static void BM_MediaSample_StructSize(benchmark::State &state) {
    for (auto _ : state) {
        auto sample = std::make_shared<media_sample>();
        sample->pts = 1000;
        sample->dts = 1000;
        sample->duration = 33;
        benchmark::DoNotOptimize(sample);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaSample_StructSize);
