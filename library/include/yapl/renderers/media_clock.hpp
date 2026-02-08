#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

namespace yapl::renderers {

/**
 * @brief Shared media clock for audio/video synchronization
 *
 * Provides a monotonic clock that both audio and video renderers use to
 * determine playback timing. Supports pause/resume and accounts for audio
 * buffer latency to maintain A/V sync.
 *
 * Thread-safe for concurrent access from multiple renderer threads.
 */
class media_clock {
  public:
    media_clock() = default;

    /**
     * @brief Start the clock from zero
     *
     * Called when first frame is ready to render. Multiple calls are idempotent.
     */
    void start() {
        if (!m_started.exchange(true)) {
            m_start_time = std::chrono::steady_clock::now();
            m_pause_offset_ms = 0;
        }
    }

    /**
     * @brief Reset clock to initial state
     *
     * Stops the clock and clears all timing state. Called on playback stop.
     */
    void reset() {
        m_started = false;
        m_paused = false;
        m_pause_offset_ms = 0;
        m_audio_latency_ms = 0;
    }

    /**
     * @brief Pause the clock
     *
     * Freezes time progression. Elapsed time during pause is excluded from playback time.
     */
    void pause() {
        if (!m_paused.exchange(true)) {
            m_pause_start = std::chrono::steady_clock::now();
        }
    }

    /**
     * @brief Resume the clock after pause
     *
     * Continues time progression, excluding the pause duration from elapsed time.
     */
    void resume() {
        if (m_paused.exchange(false)) {
            auto pause_duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - m_pause_start)
                    .count();
            m_pause_offset_ms += pause_duration;
        }
    }

    /**
     * @brief Set audio buffer latency for A/V sync
     * @param latency_ms Audio buffer size in milliseconds
     *
     * Video renderer will delay frames by this amount to stay synchronized with audio.
     */
    void set_audio_latency_ms(int64_t latency_ms) {
        m_audio_latency_ms = latency_ms;
    }

    /**
     * @brief Get current audio buffer latency
     * @return Audio latency in milliseconds
     */
    [[nodiscard]] int64_t get_audio_latency_ms() const {
        return m_audio_latency_ms.load();
    }

    /**
     * @brief Get raw playback time since start
     * @return Elapsed time in milliseconds (excluding pause durations)
     */
    [[nodiscard]] int64_t get_time_ms() const {
        if (!m_started) {
            return 0;
        }
        if (m_paused) {
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    m_pause_start - m_start_time)
                    .count();
            return elapsed - m_pause_offset_ms.load();
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - m_start_time)
                           .count();
        return elapsed - m_pause_offset_ms.load();
    }

    /**
     * @brief Get video display time accounting for audio latency
     * @return Time in milliseconds when video frames should be displayed
     *
     * This is the reference time for video frame PTS matching. Video frames
     * with PTS matching this value should be rendered now.
     */
    [[nodiscard]] int64_t get_video_time_ms() const {
        return get_time_ms() - m_audio_latency_ms.load();
    }

    /**
     * @brief Check if clock has been started
     * @return true if start() has been called
     */
    [[nodiscard]] bool is_started() const { return m_started; }

    /**
     * @brief Check if clock is currently paused
     * @return true if pause() was called without subsequent resume()
     */
    [[nodiscard]] bool is_paused() const { return m_paused; }

    /** Non-copyable, non-movable (contains atomics) */
    media_clock(const media_clock &) = delete;
    media_clock &operator=(const media_clock &) = delete;
    media_clock(media_clock &&) = delete;
    media_clock &operator=(media_clock &&) = delete;

  private:

    std::chrono::steady_clock::time_point m_start_time;
    std::chrono::steady_clock::time_point m_pause_start;
    std::atomic<int64_t> m_pause_offset_ms{0};
    std::atomic<int64_t> m_audio_latency_ms{0};
    std::atomic<bool> m_started{false};
    std::atomic<bool> m_paused{false};
};

} // namespace yapl::renderers
