
#include "yapl/media_pipeline.hpp"
#include "yapl/debug.hpp"
#include "yapl/decoders/ffmpeg/audio_decoder.hpp"
#include "yapl/decoders/ffmpeg/video_decoder.hpp"
#include "yapl/ffmpeg_media_extractor.hpp"
#include "yapl/media_info.hpp"
#include "yapl/media_pipeline.hpp"
#include "yapl/media_source.hpp"
#include "yapl/renderers/i_video_renderer_factory.hpp"
#include "yapl/renderers/i_audio_renderer_factory.hpp"
#include "yapl/track.hpp"
#include "yapl/track_info.hpp"

#include <algorithm>
#include <chrono>
#include <libavcodec/codec_id.h>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace yapl {

namespace {

inline auto get_track_type(std::span<std::shared_ptr<track>> _tracks,
                           track_type _track_type)
    -> std::optional<std::shared_ptr<track>> {
    auto _track = std::ranges::find_if(
        _tracks, [_track_type](const std::shared_ptr<track> &track) {
            return track->get_info()->type == _track_type;
        });
    if (_track != _tracks.end()) {
        return *_track;
    }
    return std::nullopt;
}

} // namespace

media_pipeline::media_pipeline(
    std::unique_ptr<renderers::i_video_renderer_factory> vrf, std::unique_ptr<renderers::i_audio_renderer_factory> arf) {
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

    m_video_render = vrf->create_video_renderer();
    m_audio_render = arf->create_audio_renderer();

    // TODO: Refactor -> media pipeline should get a decoder factory
    m_buffering_thread = std::thread([&]() {
        while (!m_data_source_eos) {
            if (!m_buffering) {
                std::unique_lock<std::mutex> lg{m_buffering_mtx};
                m_buffering_cv.wait(lg);
            }
            auto read_sample_output = m_media_extractor->read_sample();
            switch (read_sample_output.error) {
            case read_sample_error_t::no_errror: {
                auto sample = read_sample_output.sample;
                m_tracks[read_sample_output.stream_id]->push_sample(sample);

            } break;
            case read_sample_error_t::invalid_sample:
            case read_sample_error_t::invalid_packet_size:
            case read_sample_error_t::invalid_stream_index:
            case read_sample_error_t::timeout:
                LOG_ERROR("Extractor read sample failed!");
                break;
            case read_sample_error_t::end_of_stream:
                LOG_INFO("Data source reached EOS");
                m_data_source_eos = true;
                m_tracks[read_sample_output.stream_id]
                    ->set_data_source_reached_eos();
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    m_video_decoder_thread = std::thread([&]() {
        m_decoder_eos = false;
        while (!m_decoder_eos) {
            if (m_tracks.size() == 0) {
                std::this_thread::sleep_for(1s);
                continue;
            }

            static auto video_track_opt =
                get_track_type(m_tracks, track_type::video);

            if (!video_track_opt.has_value()) {
                LOG_WARN("Media stream without video!");
                return;
            }

            auto video_track = video_track_opt.value();
            auto read_sample_output = video_track->pop_sample();
            switch (read_sample_output.error) {
            case read_sample_error_t::no_errror: {
                auto sample = read_sample_output.sample;
                auto decoded = std::make_shared<media_sample>();
                decoded->duration = sample->duration;
                decoded->pts = sample->pts;
                decoded->dts = sample->dts;
                m_video_decoder->decode(m_tracks[sample->track_id]->get_info(),
                                        sample, decoded);
                if (decoded->data.size() > 0) {
                    m_video_render->push_frame(decoded);
                }
            } break;
            case read_sample_error_t::invalid_sample:
            case read_sample_error_t::invalid_packet_size:
            case read_sample_error_t::invalid_stream_index:
            case read_sample_error_t::timeout:
                LOG_ERROR("Pop sample failed!");
                break;
            case read_sample_error_t::end_of_stream:
                LOG_INFO("All frames are decoded. Stopping decoder!");
                m_decoder_eos = true;
                m_video_render->set_decoder_drained();
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    m_audio_decoder_thread = std::thread([&]() {
        LOG_INFO("Audio decoder thread started!");
        m_decoder_eos = false;
        while (!m_decoder_eos) {
            if (m_tracks.size() == 0) {
                std::this_thread::sleep_for(1s);
                continue;
            }

            static auto audio_track_opt =
                get_track_type(m_tracks, track_type::audio);

            if (!audio_track_opt.has_value()) {
                LOG_WARN("Media stream without audio!");
                return;
            }
            auto audio_track = audio_track_opt.value();
            auto read_sample_output = audio_track->pop_sample();
            switch (read_sample_output.error) {
            case read_sample_error_t::no_errror: {
                auto sample = read_sample_output.sample;
                auto decoded = std::make_shared<media_sample>();
                decoded->duration = sample->duration;
                decoded->pts = sample->pts;
                decoded->dts = sample->dts;
                m_audio_decoder->decode(m_tracks[sample->track_id]->get_info(),
                                        sample, decoded);

                m_audio_render->push_frame(decoded);
            } break;
            case read_sample_error_t::invalid_sample:
            case read_sample_error_t::invalid_packet_size:
            case read_sample_error_t::invalid_stream_index:
            case read_sample_error_t::timeout:
                LOG_ERROR("Pop sample failed!");
                break;
            case read_sample_error_t::end_of_stream:
                LOG_INFO("All frames are decoded. Stopping decoder!");
                m_decoder_eos = true;
                // m_video_render->set_decoder_drained();
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
}

media_pipeline::~media_pipeline() {
    if (m_buffering_thread.joinable()) {
        m_buffering_thread.join();
    }
    if (m_video_decoder_thread.joinable()) {
        m_video_decoder_thread.join();
    }
    if (m_audio_decoder_thread.joinable()) {
        m_audio_decoder_thread.join();
    }
}

void media_pipeline::load(const std::string_view url) {
    assert(m_media_source);
    assert(m_media_extractor);

    m_media_source->open(url);
    m_media_extractor->start();
    auto _media_info = m_media_extractor->get_media_info();
    auto _video_track_uniques =
        (*std::ranges::find_if(_media_info->tracks, [](const auto &track) {
            return track->type == track_type::video;
        }))->video.value();

    m_video_render->resize(_video_track_uniques->width,
                           _video_track_uniques->height);

    for (const auto &_track_info : _media_info->tracks) {
        LOG_DEBUG("media_pipeline - Track ID: {}, Type: {}",
                  _track_info->track_id, static_cast<int>(_track_info->type));
        m_tracks.emplace_back(std::make_shared<track>(_track_info));

        if (_track_info->type == track_type::video) {
            m_video_decoder = std::make_unique<decoders::ffmpeg::video_decoder>(
                static_cast<AVCodecID>(_track_info->codec_id),
                _track_info->video.value()->extra_data->raw_data);
        }

        if (_track_info->type == track_type::audio) {
            m_audio_decoder = std::make_unique<decoders::ffmpeg::audio_decoder>(
                static_cast<AVCodecID>(_track_info->codec_id),
                _track_info->audio.value()->extra_data->data);
        }
    }
    m_media_source->reset();
}

void media_pipeline::play() {
    LOG_INFO("media_pipeline::play");
    m_buffering = true;
    m_buffering_cv.notify_one();
    while (true) {
        m_video_render->render();
        m_audio_render->render();
        std::this_thread::sleep_for(5ms);
    }
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
