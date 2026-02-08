/**
 * @file media_clock_bench.cpp
 * @brief Performance benchmarks for media_clock
 *
 * The clock is queried on every frame render, making it a hot path.
 * These benchmarks measure the overhead of time queries.
 */

#include "yapl/renderers/media_clock.hpp"
#include <benchmark/benchmark.h>
#include <thread>
#include <mutex>

using namespace yapl::renderers;

/**
 * @brief Benchmark get_time_ms() - called on every audio/video frame
 */
static void BM_MediaClock_GetTime(benchmark::State &state) {
    media_clock clock;
    clock.start();

    for (auto _ : state) {
        auto time = clock.get_time_ms();
        benchmark::DoNotOptimize(time);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaClock_GetTime);

/**
 * @brief Benchmark get_video_time_ms() - called on every video frame render
 */
static void BM_MediaClock_GetVideoTime(benchmark::State &state) {
    media_clock clock;
    clock.start();
    clock.set_audio_latency_ms(50);

    for (auto _ : state) {
        auto time = clock.get_video_time_ms();
        benchmark::DoNotOptimize(time);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaClock_GetVideoTime);

/**
 * @brief Benchmark concurrent time queries (multiple renderer threads)
 */
static void BM_MediaClock_ConcurrentReads(benchmark::State &state) {
    static media_clock clock;
    static std::once_flag init_flag;

    std::call_once(init_flag, []() { clock.start(); });

    for (auto _ : state) {
        auto time = clock.get_time_ms();
        benchmark::DoNotOptimize(time);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaClock_ConcurrentReads)->Threads(1)->Threads(2)->Threads(4);

/**
 * @brief Benchmark pause/resume cycle
 */
static void BM_MediaClock_PauseResume(benchmark::State &state) {
    media_clock clock;
    clock.start();

    for (auto _ : state) {
        clock.pause();
        clock.resume();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaClock_PauseResume);

/**
 * @brief Benchmark audio latency updates (called every audio frame)
 */
static void BM_MediaClock_SetAudioLatency(benchmark::State &state) {
    media_clock clock;
    clock.start();

    int64_t latency = 0;
    for (auto _ : state) {
        clock.set_audio_latency_ms(latency);
        latency = (latency + 1) % 200;
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaClock_SetAudioLatency);

/**
 * @brief Benchmark is_started() check - called frequently
 */
static void BM_MediaClock_IsStarted(benchmark::State &state) {
    media_clock clock;
    clock.start();

    for (auto _ : state) {
        auto started = clock.is_started();
        benchmark::DoNotOptimize(started);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaClock_IsStarted);

/**
 * @brief Benchmark while paused (time calculation differs when paused)
 */
static void BM_MediaClock_GetTimeWhilePaused(benchmark::State &state) {
    media_clock clock;
    clock.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    clock.pause();

    for (auto _ : state) {
        auto time = clock.get_time_ms();
        benchmark::DoNotOptimize(time);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MediaClock_GetTimeWhilePaused);
