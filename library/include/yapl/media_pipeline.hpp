#pragma once

#include "yapl/decoders/idecoder.hpp"
#include "yapl/imedia_extractor.hpp"
#include "yapl/imedia_source.hpp"
#include "yapl/media_info.hpp"
#include "yapl/renderers/ivideo_renderer.hpp"
#include "yapl/track.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>

namespace yapl {

class media_pipeline {
  public:
    media_pipeline();
    ~media_pipeline();

    void load(const std::string_view url);
    void play();
    void pause();
    void buffer();
    void pause_buffering();
    void stop();
    void seek(float position);
    std::shared_ptr<media_info> get_media_info() const;
    void register_buffer_update_handler(
        std::function<void(size_t, size_t)> callback);

  private:
    bool m_playing{};
    std::condition_variable m_buffering_cv;
    std::mutex m_buffering_mtx;
    std::thread m_buffering_thread;
    std::atomic_bool m_buffering{false};
    std::atomic_bool m_data_source_eos{false};
    std::atomic_bool m_decoder_eos{false};
    std::thread m_video_decoder_thread;
    std::shared_ptr<imedia_source> m_media_source;
    std::unique_ptr<imedia_extractor> m_media_extractor;
    std::vector<std::shared_ptr<track>> m_tracks;
    std::unique_ptr<decoders::idecoder> m_video_decoder;
    std::unique_ptr<renderers::ivideo_renderer> m_video_render;
};

} // namespace yapl
