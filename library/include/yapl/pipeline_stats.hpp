#pragma once

#include <cstddef>
#include <cstdint>
#include <fmt/format.h>
#include <string>

namespace yapl {

struct queue_stats {
    size_t size{0};
    size_t capacity{0};

    [[nodiscard]] constexpr float fill_percent() const noexcept {
        return capacity > 0 ? (static_cast<float>(size) / capacity) * 100.0f
                            : 0.0f;
    }

    [[nodiscard]] std::string to_string() const {
        return std::to_string(size) + "/" + std::to_string(capacity) + " (" +
               std::to_string(static_cast<int>(fill_percent())) + "%)";
    }
};

struct progress_info {
    int64_t position_ms{0}; // Current playback position in milliseconds
    int64_t duration_ms{0}; // Total duration in milliseconds

    [[nodiscard]] constexpr float progress_percent() const noexcept {
        return duration_ms > 0
                   ? (static_cast<float>(position_ms) / duration_ms) * 100.0f
                   : 0.0f;
    }

    [[nodiscard]] std::string to_string() const {
        return format_time(position_ms) + " / " + format_time(duration_ms) +
               " (" + std::to_string(static_cast<int>(progress_percent())) +
               "%)";
    }

  private:
    [[nodiscard]] static std::string format_time(int64_t ms) {
        const auto total_secs = ms / 1000;
        const auto hours = total_secs / 3600;
        const auto mins = (total_secs % 3600) / 60;
        const auto secs = total_secs % 60;

        if (hours > 0) {
            return std::to_string(hours) + ":" + (mins < 10 ? "0" : "") +
                   std::to_string(mins) + ":" + (secs < 10 ? "0" : "") +
                   std::to_string(secs);
        }
        return std::to_string(mins) + ":" + (secs < 10 ? "0" : "") +
               std::to_string(secs);
    }
};

struct pipeline_stats {
    // Playback progress
    progress_info progress;

    // Media source buffer
    size_t media_source_buffered_bytes{0};

    // Track queues (samples waiting to be decoded)
    queue_stats video_track_queue;
    queue_stats audio_track_queue;

    // Renderer queues (decoded frames waiting to be rendered)
    queue_stats video_renderer_queue;
    queue_stats audio_renderer_queue;

    [[nodiscard]] std::string to_string() const {
        return progress.to_string() +
               " | Source: " + format_bytes(media_source_buffered_bytes) +
               " | VTrack: " + video_track_queue.to_string() +
               " | ATrack: " + audio_track_queue.to_string() +
               " | VRender: " + video_renderer_queue.to_string() +
               " | ARender: " + audio_renderer_queue.to_string();
    }

  private:
    [[nodiscard]] static std::string format_bytes(size_t bytes) {
        if (bytes >= 1024 * 1024) {
            return fmt::format("{:.2f}MB", bytes / 1024.0 / 1024.0);
        }
        if (bytes >= 1024) {
            return fmt::format("{:.2f}KB", bytes / 1024.0);
        }
        return fmt::format("{:.2f}B", static_cast<double>(bytes));
    }
};

} // namespace yapl
