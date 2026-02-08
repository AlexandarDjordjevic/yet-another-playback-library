#include "yapl/renderers/sdl/audio_renderer.hpp"
#include "yapl/detail/debug.hpp"
#include "yapl/renderers/media_clock.hpp"

#include <SDL.h>
#include <SDL_audio.h>
#include <fmt/format.h>

namespace yapl::renderers::sdl {

namespace {
// Audio format constants
constexpr int kSampleRate = 44100;
constexpr int kChannels = 2;
constexpr int kBytesPerSample = sizeof(float);
constexpr int kBytesPerSecond = kSampleRate * kChannels * kBytesPerSample;

// Convert bytes to milliseconds of audio
constexpr int64_t bytes_to_ms(uint32_t bytes) {
    return static_cast<int64_t>(bytes) * 1000 / kBytesPerSecond;
}

// Target SDL buffer size in bytes (~100ms of audio)
// This is how far ahead we queue audio
constexpr uint32_t kTargetQueueBytes = kBytesPerSecond / 10;

// Maximum SDL buffer before we stop queuing (~200ms)
constexpr uint32_t kMaxQueueBytes = kBytesPerSecond / 5;
} // namespace

audio_renderer::audio_renderer(media_clock &clock) : m_clock{clock} {
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        throw std::runtime_error("Failed to Initialize SDL audio!");
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);

    want.freq = kSampleRate;
    want.format = AUDIO_F32;
    want.channels = kChannels;
    want.samples = 1024;
    want.callback = NULL;

    m_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!m_audio_device) {
        throw std::runtime_error{
            fmt::format("SDL_OpenAudioDevice failed: {}", SDL_GetError())};
    }

    SDL_PauseAudioDevice(m_audio_device, 0);
}

audio_renderer::~audio_renderer() {
    stop();
    if (m_audio_device) {
        SDL_CloseAudioDevice(m_audio_device);
    }
    LOG_TRACE("Audio renderer destroyed");
}

void audio_renderer::push_frame(std::shared_ptr<media_sample> frame) {
    if (m_frames.is_shutdown()) {
        LOG_ERROR("Audio renderer is shutdown");
        return;
    }
    m_frames.push(frame);
}

void audio_renderer::render() {
    auto &clock = m_clock;

    if (clock.is_paused() || !clock.is_started()) {
        return;
    }

    // Check current SDL queue size
    uint32_t queued_bytes = SDL_GetQueuedAudioSize(m_audio_device);
    int64_t sdl_buffer_ms = bytes_to_ms(queued_bytes);

    // Report audio latency to clock so video can sync
    clock.set_audio_latency_ms(sdl_buffer_ms);

    // Don't queue more if buffer is full
    if (queued_bytes > kMaxQueueBytes) {
        return;
    }

    // Get next frame if we don't have one
    if (!m_pending_frame) {
        m_pending_frame = m_frames.try_pop();
        if (!m_pending_frame) {
            return;
        }
    }

    auto frame = *m_pending_frame;
    int64_t audio_playback_pos = clock.get_time_ms();

    // The audio we're about to queue will be heard after the current SDL buffer
    // So we should queue audio that has PTS = current_time + sdl_buffer_latency
    // In other words: queue if frame->pts <= audio_playback_pos + sdl_buffer_ms

    if (frame->pts > audio_playback_pos + sdl_buffer_ms + 50) {
        // Frame is too far in the future, wait
        return;
    }

    // Drop audio that's too old (would cause desync)
    if (frame->pts < audio_playback_pos - 100) {
        LOG_DEBUG("Dropping late audio. PTS: {}ms, playback: {}ms", frame->pts,
                  audio_playback_pos);
        m_pending_frame.reset();
        return;
    }

    m_pending_frame.reset();

    // Debug: log timing every second
    static int64_t last_log_time = 0;
    if (audio_playback_pos - last_log_time > 1000) {
        // Audio currently being heard = clock_time - sdl_buffer_latency
        int64_t audio_heard_now = audio_playback_pos - sdl_buffer_ms;
        LOG_INFO("[AUDIO] clock: {}ms, PTS: {}ms, SDL buf: {}ms, playing: {}ms",
                 audio_playback_pos, frame->pts, sdl_buffer_ms,
                 audio_heard_now);
        last_log_time = audio_playback_pos;
    }

    auto result = SDL_QueueAudio(m_audio_device, frame->data.data(),
                                 static_cast<uint32_t>(frame->data.size()));
    if (result < 0) {
        LOG_ERROR("SDL_QueueAudio failed: {}", SDL_GetError());
    }
}

void audio_renderer::pause() {
    if (m_audio_device) {
        SDL_PauseAudioDevice(m_audio_device, 1);
    }
    LOG_TRACE("Audio renderer paused");
}

void audio_renderer::resume() {
    if (m_audio_device) {
        SDL_PauseAudioDevice(m_audio_device, 0);
    }
    LOG_TRACE("Audio renderer resumed");
}

void audio_renderer::stop() {
    m_frames.shutdown();
    m_pending_frame.reset();
    if (m_audio_device) {
        SDL_PauseAudioDevice(m_audio_device, 1);
        SDL_ClearQueuedAudio(m_audio_device);
    }
    LOG_TRACE("Audio renderer stopped");
}

queue_stats audio_renderer::get_queue_stats() const { return m_frames.stats(); }

} // namespace yapl::renderers::sdl
