#include "yapl/player.hpp"

#include "yapl/detail/debug.hpp"
#include "yapl/decoders/ffmpeg/ffmpeg_decoder_factory.hpp"
#include "yapl/ffmpeg_media_extractor_factory.hpp"
#include "yapl/detail/media_pipeline.hpp"
#include "yapl/media_source_factory.hpp"

namespace yapl {

player::player(std::unique_ptr<i_media_source_factory> msf,
               std::unique_ptr<i_media_extractor_factory> mef,
               std::unique_ptr<decoders::i_decoder_factory> df,
               std::unique_ptr<renderers::i_video_renderer_factory> vrf,
               std::unique_ptr<renderers::i_audio_renderer_factory> arf,
               std::unique_ptr<input::i_input_handler_factory> ihf)
    : m_media_pipeline{std::make_unique<media_pipeline>(
          std::move(msf), std::move(mef), std::move(df),
          std::move(vrf), std::move(arf), std::move(ihf))} {
    LOG_DEBUG("Player initialized with custom factories");
}

player::player(std::unique_ptr<renderers::i_video_renderer_factory> vrf,
               std::unique_ptr<renderers::i_audio_renderer_factory> arf,
               std::unique_ptr<input::i_input_handler_factory> ihf)
    : m_media_pipeline{std::make_unique<media_pipeline>(
          std::make_unique<media_source_factory>(),
          std::make_unique<ffmpeg_media_extractor_factory>(),
          std::make_unique<decoders::ffmpeg::ffmpeg_decoder_factory>(),
          std::move(vrf), std::move(arf), std::move(ihf))} {
    LOG_DEBUG("Player initialized");
}

void player::load(std::string_view url) {
    LOG_DEBUG("Loading: {}", std::string(url));
    m_media_pipeline->load(url);
}

void player::play() { m_media_pipeline->play(); }

void player::pause() { m_media_pipeline->pause(); }

void player::resume() { m_media_pipeline->resume(); }

bool player::is_paused() const { return m_media_pipeline->is_paused(); }

pipeline_stats player::get_stats() const { return m_media_pipeline->get_stats(); }

void player::stop() { m_media_pipeline->stop(); }

void player::set_command_callback(input::command_callback callback) {
    m_media_pipeline->set_command_callback(std::move(callback));
}

} // namespace yapl
