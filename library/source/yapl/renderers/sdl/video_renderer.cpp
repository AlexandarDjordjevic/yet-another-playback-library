#include "yapl/renderers/sdl/video_renderer.hpp"

#include "SDL_render.h"
#include "yapl/detail/debug.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/renderers/media_clock.hpp"

namespace yapl::renderers::sdl {

namespace {
constexpr size_t kDefaultWidth = 640;
constexpr size_t kDefaultHeight = 480;
constexpr int64_t kFrameToleranceMs = 15;
} // namespace

video_renderer::video_renderer(media_clock &clock)
    : m_clock{clock}, m_width{kDefaultWidth}, m_height{kDefaultHeight} {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Failed to Initialize SDL video!");
    }

    m_window =
        SDL_CreateWindow("YAPL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         static_cast<int>(m_width), static_cast<int>(m_height),
                         SDL_WINDOW_SHOWN);

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    m_texture = SDL_CreateTexture(
        m_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(m_width), static_cast<int>(m_height));
}

video_renderer::~video_renderer() {
    stop();
    if (m_texture)
        SDL_DestroyTexture(m_texture);
    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);
    if (m_window)
        SDL_DestroyWindow(m_window);
    LOG_TRACE("Video renderer destroyed");
}

void video_renderer::resize(size_t width, size_t height) {
    m_width = width;
    m_height = height;

    if (m_texture)
        SDL_DestroyTexture(m_texture);
    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);
    if (m_window)
        SDL_DestroyWindow(m_window);

    m_window =
        SDL_CreateWindow("YAPL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         static_cast<int>(m_width), static_cast<int>(m_height),
                         SDL_WINDOW_SHOWN);

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    m_texture = SDL_CreateTexture(
        m_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(m_width), static_cast<int>(m_height));
}

void video_renderer::push_frame(std::shared_ptr<media_sample> frame) {
    if (m_frames.is_shutdown()) {
        LOG_ERROR("Video renderer is shutdown");
        return;
    }
    m_frames.push(frame);
}

void video_renderer::pause() {
    m_clock.pause();
    LOG_TRACE("Video renderer paused");
}

void video_renderer::resume() {
    m_clock.resume();
    LOG_TRACE("Video renderer resumed");
}

void video_renderer::stop() {
    m_frames.shutdown();
    m_pending_frame.reset();
    m_clock.reset();
    LOG_TRACE("Video renderer stopped");
}

void video_renderer::render() {
    auto &clock = m_clock;

    if (m_frames.is_empty() || clock.is_paused()) {
        return;
    }

    // Start the clock when first frame is ready
    if (!clock.is_started()) {
        clock.start();
    }

    if (!m_pending_frame) {
        m_pending_frame = m_frames.try_pop();
        if (!m_pending_frame) {
            return;
        }
    }

    auto frame = *m_pending_frame;
    const auto video_time_ms = clock.get_video_time_ms();

    if (frame->pts > video_time_ms + kFrameToleranceMs) {
        return;
    }

    m_pending_frame.reset();

    if (frame->pts < video_time_ms - kFrameToleranceMs) {
        LOG_DEBUG("Dropping late frame. PTS: {}ms, video_time: {}ms",
                  frame->pts, video_time_ms);
        m_current_position_ms = frame->pts;
        return;
    }

    m_current_position_ms = frame->pts;

    // Debug: log timing every second
    static int64_t last_log_time = 0;
    if (video_time_ms - last_log_time > 1000) {
        LOG_INFO(
            "[VIDEO] video_time: {}ms, PTS: {}ms, diff: {}ms, audio_lat: {}ms",
            video_time_ms, frame->pts, video_time_ms - frame->pts,
            clock.get_audio_latency_ms());
        last_log_time = video_time_ms;
    }

    const int y_pitch = static_cast<int>(m_width);
    const int u_pitch = static_cast<int>(m_width / 2);
    const int v_pitch = static_cast<int>(m_width / 2);

    uint8_t *const y_plane = frame->data.data();
    uint8_t *const u_plane = y_plane + m_width * m_height;
    uint8_t *const v_plane = u_plane + (m_width * m_height) / 4;

    SDL_UpdateYUVTexture(m_texture, nullptr, y_plane, y_pitch, u_plane, u_pitch,
                         v_plane, v_pitch);

    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}

queue_stats video_renderer::get_queue_stats() const { return m_frames.stats(); }

int64_t video_renderer::get_current_position_ms() const {
    return m_current_position_ms.load();
}

} // namespace yapl::renderers::sdl
