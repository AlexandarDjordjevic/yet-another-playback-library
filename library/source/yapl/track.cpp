#include "yapl/track.hpp"
#include "yapl/blocking_queue.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"
#include <memory>

using namespace std::chrono_literals;

namespace yapl {

track::track(std::shared_ptr<track_info> info)
    : m_track_info{info}, m_data_source_eos_reached{false},
      m_buffered_duration{0},
      m_sample_queue{blocking_queue<std::shared_ptr<media_sample>>(1024)} {}

void track::push_sample(const std::shared_ptr<media_sample> sample) {
    m_buffered_duration += sample->duration;
    m_sample_queue.push(sample);
}

void track::set_data_source_reached_eos() { m_data_source_eos_reached = true; }

std::shared_ptr<track_info> track::get_info() const { return m_track_info; }

read_sample_result track::pop_sample() {
    if (m_sample_queue.is_empty() && m_data_source_eos_reached) {
        return {.stream_id = m_track_info->track_id,
                .error = read_sample_error_t::end_of_stream,
                .sample{}};
    }

    auto data = m_sample_queue.pop();
    return {.stream_id = m_track_info->track_id,
            .error = read_sample_error_t::no_errror,
            .sample = data};
}

} // namespace yapl
