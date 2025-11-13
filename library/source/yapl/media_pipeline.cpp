
#include "yapl/media_pipeline.hpp"
#include "yapl/debug.hpp"
#include "yapl/decoders/ffmpeg/video_decoder.hpp"
#include "yapl/ffmpeg_media_extractor.hpp"
#include "yapl/media_info.hpp"
#include "yapl/media_pipeline.hpp"
#include "yapl/media_source.hpp"
#include "yapl/renderers/sdl/video_renderer.hpp"
#include "yapl/track.hpp"
#include "yapl/track_info.hpp"

#include <algorithm>
#include <chrono>
#include <libavcodec/codec_id.h>
#include <memory>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

namespace yapl {

media_pipeline::media_pipeline() {
    m_video_render = std::make_unique<renderers::sdl::video_renderer>(640, 360);
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
    m_buffering_thread = std::thread([this]() {
        while (!m_eos_reached) {
            if (!m_buffering) {
                std::unique_lock<std::mutex> lg{m_buffering_mtx};
                m_buffering_cv.wait(lg);
            }
            auto read_sample_output = m_media_extractor->read_sample();
            switch (read_sample_output.error) {

            case read_sample_error_t::no_errror: {
                auto sample = read_sample_output.sample;
                LOG_DEBUG(
                    "media_pipeline - Sample read: trackId: {}, pts: {}, dts "
                    ": {} duration: {}",
                    sample->track_id, sample->pts, sample->dts,
                    sample->duration);

                m_tracks[sample->track_id]->push_sample(sample);
            } break;
            case read_sample_error_t::invalid_sample:
            case read_sample_error_t::invalid_packet_size:
            case read_sample_error_t::invalid_stream_index:
                break;
            case read_sample_error_t::end_of_stream:
                m_eos_reached = true;
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
            auto decoded = std::make_shared<media_sample>();

            m_video_decoder->decode(m_tracks[sample->track_id]->get_info(),
                                    sample, decoded);
            // HACK
            if (decoded->data.size() > 0) {
                m_video_render->push_frame(decoded);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}

media_pipeline::~media_pipeline() {
    if (m_buffering_thread.joinable()) {
        m_buffering_thread.join();
    }
}

void media_pipeline::load(const std::string_view url) {
    assert(m_media_source);
    assert(m_media_extractor);

    m_media_source->open(url);
    m_media_extractor->start();
    auto _media_info = m_media_extractor->get_media_info();
    for (const auto &_track_info : _media_info->tracks) {
        LOG_DEBUG("media_pipeline - Track ID: {}, Type: {}",
                  _track_info->track_id, static_cast<int>(_track_info->type));
        m_tracks.emplace_back(std::make_shared<track>(_track_info));

        if (_track_info->type == track_type::video) {
            m_video_decoder = std::make_unique<decoders::ffmpeg::video_decoder>(
                static_cast<AVCodecID>(_track_info->codec_id));
        }
    }
    m_media_source->reset();
}

void media_pipeline::play() {
    LOG_INFO("media_pipeline::play");
    m_buffering = true;
    m_buffering_cv.notify_one();
    m_video_render->render();
}

void media_pipeline::pause() { m_buffering = false; }

void media_pipeline::stop() {
    // Implement stop logic
}

void media_pipeline::buffer() {
    m_buffering = true;
    m_buffering_cv.notify_one();
}

void media_pipeline::pause_buffering() { m_buffering = false; }

void media_pipeline::seek([[maybe_unused]] float position) {
    // Implement seek logic
}

std::shared_ptr<media_info> media_pipeline::get_media_info() const {
    return m_media_extractor->get_media_info();
}

void media_pipeline::register_buffer_update_handler(
    [[maybe_unused]] std::function<void(size_t, size_t)> callback) {}

} // namespace yapl
