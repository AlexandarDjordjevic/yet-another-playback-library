#pragma once

#include "yapl/blocking_queue.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"
#include <memory>

namespace yapl {

struct track {
  public:
    explicit track(std::shared_ptr<track_info> info);
    void push_sample(std::shared_ptr<media_sample> sample);
    std::shared_ptr<media_sample> pop_sample();
    [[nodiscard]] std::shared_ptr<track_info> get_info() const;

  private:
    std::shared_ptr<track_info> m_track_info;
    size_t m_buffered_duration;
    blocking_queue<std::shared_ptr<media_sample>> m_sample_queue;
};

} // namespace yapl
