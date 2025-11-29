#include "yapl/detail/media_pipeline.hpp"
#include "yapl/detail/debug.hpp"
#include "yapl/media_info.hpp"
#include "yapl/track.hpp"
#include "yapl/track_info.hpp"

#include <chrono>

using namespace std::chrono_literals;

namespace yapl {

namespace {
constexpr auto kThreadSleep = 1ms;
constexpr auto kRenderSleep = 5ms;
} // namespace

media_pipeline::media_pipeline(
    std::unique_ptr<i_media_source_factory> msf,
    std::unique_ptr<i_media_extractor_factory> mef,
    std::unique_ptr<decoders::i_decoder_factory> df,
    std::unique_ptr<renderers::i_video_renderer_factory> vrf,
    std::unique_ptr<renderers::i_audio_renderer_factory> arf,
    std::unique_ptr<input::i_input_handler_factory> ihf)
    : m_media_source_factory{std::move(msf)},
      m_media_extractor_factory{std::move(mef)},
      m_decoder_factory{std::move(df)},
      m_media_source{m_media_source_factory->create()},
      m_media_extractor{m_media_extractor_factory->create(m_media_source)},
      m_video_render{vrf->create_video_renderer()},
      m_audio_render{arf->create_audio_renderer()},
      m_input_handler{ihf->create()} {}

media_pipeline::~media_pipeline() {
    stop();
    LOG_TRACE("Media pipeline destroyed");
}

void media_pipeline::load(std::string_view url) {
    LOG_INFO("Loading media: {}", std::string(url));

    m_media_source->open(url);
    m_media_extractor->start();

    auto media_info = m_media_extractor->get_media_info();

    m_tracks.clear();
    m_video_track = nullptr;
    m_audio_track = nullptr;

    for (const auto &track_info : media_info->tracks) {
        LOG_DEBUG("Track ID: {}, Type: {}", track_info->track_id,
                  track_type_to_string(track_info->type));

        auto new_track = std::make_shared<track>(track_info);
        m_tracks.emplace_back(new_track);

        if (track_info->type == track_type::video && !m_video_track) {
            m_video_track = new_track;
            m_video_decoder = m_decoder_factory->create_video_decoder(
                track_info->codec_id,
                track_info->video.value()->extra_data->raw_data);
            m_video_render->resize(track_info->video.value()->width,
                                   track_info->video.value()->height);
        }

        if (track_info->type == track_type::audio && !m_audio_track) {
            m_audio_track = new_track;
            m_audio_decoder = m_decoder_factory->create_audio_decoder(
                track_info->codec_id,
                track_info->audio.value()->extra_data->data);
        }
    }

    LOG_INFO("Media loaded successfully");
}

void media_pipeline::play() {
    LOG_DEBUG("Playback starting");
    m_running = true;
    m_paused = false;

    m_buffering_thread = std::jthread([this](std::stop_token st) {
        LOG_DEBUG("Buffering thread started");
        while (!st.stop_requested()) {
            if (m_paused) {
                std::this_thread::sleep_for(kThreadSleep);
                continue;
            }

            auto result = m_media_extractor->read_sample();
            if (result.error == read_sample_error_t::no_errror) {
                if (result.stream_id < m_tracks.size()) {
                    m_tracks[result.stream_id]->push_sample(result.sample);
                }
            } else if (result.error == read_sample_error_t::end_of_stream) {
                LOG_DEBUG("Buffering: EOS reached");
                for (auto &t : m_tracks) {
                    t->set_data_source_reached_eos();
                }
                break;
            }
            std::this_thread::sleep_for(kThreadSleep);
        }
        LOG_DEBUG("Buffering thread exiting");
    });

    if (m_video_track) {
        m_video_decoder_thread = std::jthread([this](std::stop_token st) {
            LOG_DEBUG("Video decoder thread started");
            while (!st.stop_requested()) {
                if (m_paused) {
                    std::this_thread::sleep_for(kThreadSleep);
                    continue;
                }

                auto result = m_video_track->pop_sample();
                if (result.error == read_sample_error_t::no_errror) {
                    auto decoded = std::make_shared<media_sample>();
                    decoded->duration = result.sample->duration;
                    decoded->pts = result.sample->pts;
                    decoded->dts = result.sample->dts;
                    m_video_decoder->decode(m_video_track->get_info(),
                                            result.sample, decoded);
                    if (!decoded->data.empty()) {
                        m_video_render->push_frame(decoded);
                    }
                } else if (result.error == read_sample_error_t::end_of_stream) {
                    LOG_DEBUG("Video decoder: EOS reached");
                    break;
                }
                std::this_thread::sleep_for(kThreadSleep);
            }
            LOG_DEBUG("Video decoder thread exiting");
        });
    }

    if (m_audio_track) {
        m_audio_decoder_thread = std::jthread([this](std::stop_token st) {
            LOG_DEBUG("Audio decoder thread started");
            while (!st.stop_requested()) {
                if (m_paused) {
                    std::this_thread::sleep_for(kThreadSleep);
                    continue;
                }

                auto result = m_audio_track->pop_sample();
                if (result.error == read_sample_error_t::no_errror) {
                    auto decoded = std::make_shared<media_sample>();
                    decoded->duration = result.sample->duration;
                    decoded->pts = result.sample->pts;
                    decoded->dts = result.sample->dts;
                    m_audio_decoder->decode(m_audio_track->get_info(),
                                            result.sample, decoded);
                    m_audio_render->push_frame(decoded);
                } else if (result.error == read_sample_error_t::end_of_stream) {
                    LOG_DEBUG("Audio decoder: EOS reached");
                    break;
                }
                std::this_thread::sleep_for(kThreadSleep);
            }
            LOG_DEBUG("Audio decoder thread exiting");
        });
    }

    while (m_running) {
        m_input_handler->poll();

        if (!m_paused) {
            m_video_render->render();
            m_audio_render->render();
        }
        std::this_thread::sleep_for(kRenderSleep);
    }
}

void media_pipeline::pause() {
    LOG_DEBUG("Playback paused");
    m_paused = true;
    if (m_video_render)
        m_video_render->pause();
    if (m_audio_render)
        m_audio_render->pause();
}

void media_pipeline::resume() {
    LOG_DEBUG("Playback resumed");
    m_paused = false;
    if (m_video_render)
        m_video_render->resume();
    if (m_audio_render)
        m_audio_render->resume();
}

bool media_pipeline::is_paused() const { return m_paused; }

void media_pipeline::stop() {
    LOG_DEBUG("Playback stopping");
    m_running = false;

    for (auto &t : m_tracks) {
        t->shutdown();
    }

    if (m_video_render)
        m_video_render->stop();
    if (m_audio_render)
        m_audio_render->stop();
}

std::shared_ptr<media_info> media_pipeline::get_media_info() const {
    return m_media_extractor->get_media_info();
}

pipeline_stats media_pipeline::get_stats() const {
    pipeline_stats stats;

    if (m_video_render) {
        stats.progress.position_ms = m_video_render->get_current_position_ms();
    }
    if (m_media_extractor) {
        auto info = m_media_extractor->get_media_info();
        if (info) {
            stats.progress.duration_ms =
                static_cast<int64_t>(info->duration / 1000);
        }
    }

    stats.media_source_buffered_bytes = m_media_source->available();

    if (m_video_track) {
        stats.video_track_queue = m_video_track->get_queue_stats();
    }
    if (m_audio_track) {
        stats.audio_track_queue = m_audio_track->get_queue_stats();
    }
    if (m_video_render) {
        stats.video_renderer_queue = m_video_render->get_queue_stats();
    }
    if (m_audio_render) {
        stats.audio_renderer_queue = m_audio_render->get_queue_stats();
    }

    return stats;
}

void media_pipeline::set_command_callback(input::command_callback callback) {
    m_input_handler->set_command_callback(std::move(callback));
}

} // namespace yapl
