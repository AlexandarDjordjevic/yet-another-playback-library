#include "sdl/audio_renderer.hpp"
#include "yapl/debug.hpp"

#include <SDL.h>
#include <SDL_audio.h>
#include <cstdio>
#include <cstdlib>
#include <fmt/format.h>

namespace renderers::sdl {

audio_renderer::audio_renderer() : m_running{false}, m_decoder_drained{false} {
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        throw("Failed to Initialize SDL library!");
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);

    want.freq = 44100;       // e.g., 44100 or decoder->sample_rate
    want.format = AUDIO_F32; // float, interleaved
    want.channels = 2;       // stereo
    want.samples = 1024;     // buffer size
    want.callback = NULL;    // we will use SDL_QueueAudio()

    m_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!m_audio_device) {
        LOG_CRITICAL("SDL_OpenAudioDevice failed: {}", SDL_GetError());
        throw std::runtime_error{
            fmt::format("SDL_OpenAudioDevice failed {}", SDL_GetError())};
    }

    SDL_PauseAudioDevice(m_audio_device, 0); // start playback
}

audio_renderer::~audio_renderer() { m_running = false; }

void audio_renderer::push_frame(std::shared_ptr<yapl::media_sample> frame) {
    m_frames.push(frame);
}

void audio_renderer::render() {
    auto frame = m_frames.try_pop();
    if (!frame.has_value()) {
        return;
    }
    auto result =
        SDL_QueueAudio(m_audio_device, frame.value()->data.data(),
                       static_cast<uint32_t>(frame.value()->data.size()));
    if (result < 0) {
        throw std::runtime_error{
            fmt::format("SDL_QueueAudio failed {}", SDL_GetError())};
    }
}

} // namespace renderers::sdl
