#include "yapl/renderers/media_clock.hpp"
#include <gtest/gtest.h>
#include <thread>

using namespace yapl::renderers;

TEST(MediaClockTest, InitialState) {
    media_clock clock;

    EXPECT_FALSE(clock.is_started());
    EXPECT_FALSE(clock.is_paused());
    EXPECT_EQ(clock.get_time_ms(), 0);
    EXPECT_EQ(clock.get_audio_latency_ms(), 0);
}

TEST(MediaClockTest, StartSetsTime) {
    media_clock clock;

    clock.start();

    EXPECT_TRUE(clock.is_started());
    EXPECT_FALSE(clock.is_paused());

    // Time should advance
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_GT(clock.get_time_ms(), 5);
}

TEST(MediaClockTest, PauseStopsTime) {
    media_clock clock;

    clock.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto time_before_pause = clock.get_time_ms();
    clock.pause();

    EXPECT_TRUE(clock.is_paused());

    // Wait and verify time doesn't advance
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto time_during_pause = clock.get_time_ms();

    EXPECT_NEAR(time_before_pause, time_during_pause, 2);
}

TEST(MediaClockTest, ResumeRestoresTime) {
    media_clock clock;

    clock.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    clock.pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    clock.resume();

    EXPECT_FALSE(clock.is_paused());

    // Time should advance again
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_GT(clock.get_time_ms(), 15);
}

TEST(MediaClockTest, ResetClearsState) {
    media_clock clock;

    clock.start();
    clock.set_audio_latency_ms(100);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    clock.reset();

    EXPECT_FALSE(clock.is_started());
    EXPECT_FALSE(clock.is_paused());
    EXPECT_EQ(clock.get_time_ms(), 0);
    EXPECT_EQ(clock.get_audio_latency_ms(), 0);
}

TEST(MediaClockTest, AudioLatencyAffectsVideoTime) {
    media_clock clock;

    clock.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto time_ms = clock.get_time_ms();
    auto video_time_no_latency = clock.get_video_time_ms();

    EXPECT_NEAR(time_ms, video_time_no_latency, 2);

    clock.set_audio_latency_ms(50);

    auto video_time_with_latency = clock.get_video_time_ms();

    // Video time should be 50ms behind clock time
    EXPECT_NEAR(time_ms - 50, video_time_with_latency, 2);
}

TEST(MediaClockTest, GetAudioLatency) {
    media_clock clock;

    EXPECT_EQ(clock.get_audio_latency_ms(), 0);

    clock.set_audio_latency_ms(75);
    EXPECT_EQ(clock.get_audio_latency_ms(), 75);

    clock.set_audio_latency_ms(0);
    EXPECT_EQ(clock.get_audio_latency_ms(), 0);
}

TEST(MediaClockTest, MultipleStartCallsIdempotent) {
    media_clock clock;

    clock.start();
    auto time1 = clock.get_time_ms();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Second start should not reset the clock
    clock.start();
    auto time2 = clock.get_time_ms();

    EXPECT_GT(time2, time1);
}

TEST(MediaClockTest, PauseAndResumePreservesElapsedTime) {
    media_clock clock;

    clock.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    clock.pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    clock.resume();

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    auto time_after_resume = clock.get_time_ms();

    // Total elapsed should be ~40ms (20 + 20), not including pause duration
    EXPECT_NEAR(time_after_resume, 40, 10);
}
