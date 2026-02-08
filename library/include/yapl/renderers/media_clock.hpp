#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

namespace yapl::renderers {

/// Shared media clock for A/V synchronization.
/// Both audio and video renderers use this to determine playback timing.
class media_clock {
  public:
    media_clock() = default;

    /// Start the clock (called when first frame is ready to render)
    void start() {
        if (!m_started.exchange(true)) {
            m_start_time = std::chrono::steady_clock::now();
            m_pause_offset_ms = 0;
        }
    }

    /// Reset the clock (called on stop)
    void reset() {
        m_started = false;
        m_paused = false;
        m_pause_offset_ms = 0;
        m_audio_latency_ms = 0;
    }

    /// Pause the clock
    void pause() {
        if (!m_paused.exchange(true)) {
            m_pause_start = std::chrono::steady_clock::now();
        }
    }

    /// Resume the clock
    void resume() {
        if (m_paused.exchange(false)) {
            auto pause_duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - m_pause_start)
                    .count();
            m_pause_offset_ms += pause_duration;
        }
    }

    /// Set the audio buffer latency (SDL queue size in ms)
    /// Video will delay by this amount to stay in sync with audio
    void set_audio_latency_ms(int64_t latency_ms) {
        m_audio_latency_ms = latency_ms;
    }

    /// Get current audio buffer latency
    [[nodiscard]] int64_t get_audio_latency_ms() const {
        return m_audio_latency_ms.load();
    }

    /// Get current playback time in milliseconds (raw clock)
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

    /// Get video display time (accounts for audio latency)
    /// Video should display frames when their PTS matches this time
    [[nodiscard]] int64_t get_video_time_ms() const {
        return get_time_ms() - m_audio_latency_ms.load();
    }

    [[nodiscard]] bool is_started() const { return m_started; }
    [[nodiscard]] bool is_paused() const { return m_paused; }

    // Non-copyable, non-movable (contains atomics)
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
