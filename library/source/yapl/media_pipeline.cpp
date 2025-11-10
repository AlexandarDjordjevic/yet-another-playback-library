
#include "yapl/media_pipeline.hpp"
#include "yapl/debug.hpp"
#include "yapl/decoders/ffmpeg/video_decoder.hpp"
#include "yapl/ffmpeg_media_extractor.hpp"
#include "yapl/media_info.hpp"
#include "yapl/media_pipeline.hpp"
#include "yapl/media_source.hpp"
#include "yapl/track.hpp"
#include "yapl/track_info.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

namespace yapl {

media_pipeline::media_pipeline() {
    // TODO: Refactor -> Media pipeline should get a media source factory
    m_media_source = std::make_unique<media_source>();
    if (!m_media_source) {
        throw std::runtime_error("Media source is not created!");
    }

    m_media_extractor =
        std::make_unique<ffmpeg_media_extractor>(m_media_source);
    if (!m_media_extractor) {
        throw std::runtime_error("Failed to construct media extractor!");
    }

    // TODO: Refactor -> media pipeline should get a decoder factory
    m_video_decoder = std::make_unique<decoders::ffmpeg::video_decoder>();

    m_buffering_thread = std::thread([this]() {
        while (true) {
            if (!m_buffering) {
                std::unique_lock<std::mutex> lg{m_buffering_mtx};
                m_buffering_cv.wait(lg);
            }
            auto sample = m_media_extractor->read_sample();
            LOG_DEBUG("media_pipeline - Sample read: trackId: {}, pts: {}, dts "
                      ": {} duration: {}",
                      sample->track_id, sample->pts, sample->dts,
                      sample->duration);

            m_tracks[sample->track_id]->push_sample(sample);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    m_video_decoder_thread = std::thread([&]() {
        while (true) {
            if (m_tracks.size() == 0) {
                std::this_thread::sleep_for(1s);
                continue;
            }
            auto video_track = *std::ranges::find_if(
                m_tracks, [](const std::shared_ptr<track> &track) {
                    return track->get_info()->type == track_type::video;
                });

            auto sample = video_track->pop_sample();
            LOG_DEBUG(
                "Video decoder thread - pop sample dts {}, pts {}, duration {}",
                sample->dts, sample->pts, sample->duration);
            auto decoded = std::make_shared<decoded_media_sample>();

            m_video_decoder->decode(m_tracks[sample->track_id]->get_info(),
                                    sample, decoded);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    LOG_INFO("media_pipeline initialized");
}

media_pipeline::~media_pipeline() {
    if (m_buffering_thread.joinable()) {
        m_buffering_thread.join();
    }
    LOG_INFO("media_pipeline destructor called");
}

void media_pipeline::load(const std::string_view url) {
    LOG_DEBUG("media_pipeline - loading URL: {}", std::string(url));
    assert(m_media_source);
    assert(m_media_extractor);

    m_media_source->open(url);
    m_media_extractor->start();
    m_media_info = m_media_extractor->get_media_info();
    for (const auto &_track_info : m_media_info.tracks) {
        LOG_DEBUG("media_pipeline - Track ID: {}, Type: {}",
                  _track_info->track_id, static_cast<int>(_track_info->type));
        m_tracks.emplace_back(std::make_shared<track>(_track_info));
    }
}

void media_pipeline::play() {
    LOG_INFO("media_pipeline playing");
    m_buffering = true;
    m_buffering_cv.notify_one();
}

void media_pipeline::pause() { m_buffering = false; }

void media_pipeline::stop() {
    // Implement stop logic
}

void media_pipeline::buffer() {
    LOG_INFO("media_pipeline buffering");
    m_buffering = true;
    m_buffering_cv.notify_one();
}

void media_pipeline::pause_buffering() {
    LOG_INFO("media_pipeline pause buffering");
    m_buffering = false;
}

void media_pipeline::seek([[maybe_unused]] float position) {
    // Implement seek logic
}

media_info media_pipeline::get_media_info() const { return m_media_info; }

void media_pipeline::register_buffer_update_handler(
    [[maybe_unused]] std::function<void(size_t, size_t)> callback) {}

} // namespace yapl
