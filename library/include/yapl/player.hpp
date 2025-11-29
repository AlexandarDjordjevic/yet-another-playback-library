#pragma once

#include "yapl/decoders/i_decoder_factory.hpp"
#include "yapl/i_media_extractor_factory.hpp"
#include "yapl/i_media_source_factory.hpp"
#include "yapl/input/i_input_handler.hpp"
#include "yapl/input/i_input_handler_factory.hpp"
#include "yapl/detail/media_pipeline.hpp"

#include <string_view>

namespace yapl {

class player {
  public:
    // Full constructor with all factories
    player(std::unique_ptr<i_media_source_factory> msf,
           std::unique_ptr<i_media_extractor_factory> mef,
           std::unique_ptr<decoders::i_decoder_factory> df,
           std::unique_ptr<renderers::i_video_renderer_factory> vrf,
           std::unique_ptr<renderers::i_audio_renderer_factory> arf,
           std::unique_ptr<input::i_input_handler_factory> ihf);

    // Convenience constructor using default implementations
    player(std::unique_ptr<renderers::i_video_renderer_factory> vrf,
           std::unique_ptr<renderers::i_audio_renderer_factory> arf,
           std::unique_ptr<input::i_input_handler_factory> ihf);

    ~player() = default;

    void load(std::string_view url);
    void play();
    void pause();
    void resume();
    void stop();
    [[nodiscard]] bool is_paused() const;
    [[nodiscard]] pipeline_stats get_stats() const;
    void set_command_callback(input::command_callback callback);

  private:
    std::unique_ptr<media_pipeline> m_media_pipeline;
};

} // namespace yapl
