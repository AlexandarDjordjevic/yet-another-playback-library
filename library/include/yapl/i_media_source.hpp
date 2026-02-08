#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace yapl {

/**
 * @brief Interface for media data source abstraction.
 *
 * This interface provides a unified way to access media data from various sources
 * such as files, network streams, or memory buffers. It handles low-level data
 * reading and buffering operations.
 */
struct i_media_source {
    virtual ~i_media_source() = default;

    /**
     * @brief Opens a media source from the specified URL.
     *
     * @param url The URL or path to the media source (e.g., file path, network URL)
     * @throws std::runtime_error if the source cannot be opened
     */
    virtual void open(const std::string_view url) = 0;

    /**
     * @brief Closes the media source and releases associated resources.
     */
    virtual void close() = 0;

    /**
     * @brief Reads a packet of data from the media source.
     *
     * @param size The requested number of bytes to read
     * @param buffer The buffer to write the data into
     * @return The actual number of bytes read (may be less than requested)
     * @throws std::runtime_error if a read error occurs
     */
    virtual size_t read_packet(size_t size, std::span<uint8_t> buffer) = 0;

    /**
     * @brief Returns the number of bytes currently available to read.
     *
     * @return The number of bytes available in the source
     */
    virtual size_t available() const = 0;

    /**
     * @brief Resets the media source to its initial position.
     *
     * @throws std::runtime_error if reset is not supported or fails
     */
    virtual void reset() = 0;
};

} // namespace yapl
