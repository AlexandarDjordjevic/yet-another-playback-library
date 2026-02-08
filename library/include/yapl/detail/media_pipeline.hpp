#pragma once

#include "yapl/decoders/i_decoder.hpp"
#include "yapl/decoders/i_decoder_factory.hpp"
#include "yapl/i_media_extractor.hpp"
#include "yapl/i_media_extractor_factory.hpp"
#include "yapl/i_media_source.hpp"
#include "yapl/i_media_source_factory.hpp"
#include "yapl/input/i_input_handler.hpp"
#include "yapl/input/i_input_handler_factory.hpp"
#include "yapl/media_info.hpp"
#include "yapl/pipeline_config.hpp"
#include "yapl/pipeline_stats.hpp"
#include "yapl/renderers/i_audio_renderer.hpp"
#include "yapl/renderers/i_audio_renderer_factory.hpp"
#include "yapl/renderers/i_video_renderer.hpp"
#include "yapl/renderers/i_video_renderer_factory.hpp"
#include "yapl/renderers/media_clock.hpp"
#include "yapl/track.hpp"

#include <atomic>
#include <thread>

namespace yapl {

class media_pipeline {
  public:
    media_pipeline(std::unique_ptr<i_media_source_factory> msf,
                   std::unique_ptr<i_media_extractor_factory> mef,
                   std::unique_ptr<decoders::i_decoder_factory> df,
                   std::unique_ptr<renderers::i_video_renderer_factory> vrf,
                   std::unique_ptr<renderers::i_audio_renderer_factory> arf,
                   std::unique_ptr<input::i_input_handler_factory> ihf,
                   pipeline_config config = {});
    ~media_pipeline();

    media_pipeline() = delete;
    media_pipeline(const media_pipeline &) = delete;
    media_pipeline operator=(const media_pipeline &) = delete;
    media_pipeline(media_pipeline &&) = delete;
    media_pipeline operator=(media_pipeline &&) = delete;

    void load(std::string_view url);
    void play();
    void pause();
    void resume();
    void stop();
    [[nodiscard]] bool is_paused() const;
    [[nodiscard]] std::shared_ptr<media_info> get_media_info() const;
    [[nodiscard]] pipeline_stats get_stats() const;
    void set_command_callback(input::command_callback callback);

  private:
    std::unique_ptr<i_media_source_factory> m_media_source_factory;
    std::unique_ptr<i_media_extractor_factory> m_media_extractor_factory;
    std::unique_ptr<decoders::i_decoder_factory> m_decoder_factory;
    pipeline_config m_config;

    std::atomic_bool m_running{false};
    std::atomic_bool m_paused{false};

    renderers::media_clock m_media_clock;
    std::shared_ptr<i_media_source> m_media_source;
    std::unique_ptr<i_media_extractor> m_media_extractor;
    std::vector<std::shared_ptr<track>> m_tracks;
    std::shared_ptr<track> m_video_track;
    std::shared_ptr<track> m_audio_track;
    std::unique_ptr<decoders::i_decoder> m_video_decoder;
    std::unique_ptr<decoders::i_decoder> m_audio_decoder;
    std::unique_ptr<renderers::i_video_renderer> m_video_render;
    std::unique_ptr<renderers::i_audio_renderer> m_audio_render;
    std::unique_ptr<input::i_input_handler> m_input_handler;

    std::jthread m_buffering_thread;
    std::jthread m_video_decoder_thread;
    std::jthread m_audio_decoder_thread;
};

} // namespace yapl
