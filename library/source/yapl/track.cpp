#include "yapl/track.hpp"
#include "yapl/blocking_queue.hpp"
#include "yapl/debug.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"
#include <memory>

namespace yapl {

track::track(const std::shared_ptr<track_info> info)
    : m_track_info{info}, m_buffered_duration{0},
      m_sample_queue{blocking_queue<std::shared_ptr<media_sample>>(1024)} {}

void track::push_sample(const std::shared_ptr<media_sample> sample) {
    // Process the sample
    // For example, you can store it in a buffer or send it to a decoder
    // For now, we will just print the sample information
    m_buffered_duration += sample->duration;

    LOG_DEBUG("track - Sample pushed: trackId: {}, pts: {}, dts: {}, duration: "
              "{}, buffered duration: {}",
              sample->track_id, sample->pts, sample->dts, sample->duration,
              m_buffered_duration);

    m_sample_queue.push(sample);
}

std::shared_ptr<track_info> track::get_info() const { return m_track_info; }

std::shared_ptr<media_sample> track::pop_sample() {
    return m_sample_queue.pop();
}

} // namespace yapl
