#pragma once

#include "yapl/media_info.hpp"
#include "yapl/media_sample.hpp"

namespace yapl {

/**
 * @brief Interface for extracting and demultiplexing media data.
 *
 * This interface represents a media demuxer that extracts individual samples
 * (frames, packets) from a media container format. It provides access to
 * media metadata and enables sequential reading of media samples.
 */
struct i_media_extractor {
    virtual ~i_media_extractor() = default;

    /**
     * @brief Starts the extraction process.
     *
     * This method initializes the extractor and prepares it for reading samples.
     * Must be called before calling read_sample().
     *
     * @throws std::runtime_error if the extractor cannot be started
     */
    virtual void start() = 0;

    /**
     * @brief Retrieves information about the media content.
     *
     * @return A shared pointer to media_info containing metadata about tracks,
     *         codecs, duration, and other media properties
     */
    virtual std::shared_ptr<media_info> get_media_info() const = 0;

    /**
     * @brief Reads the next media sample from the source.
     *
     * @return A read_sample_result containing the sample data and status information
     * @throws std::runtime_error if a critical read error occurs
     */
    virtual read_sample_result read_sample() = 0;
};

} // namespace yapl
