#pragma once

#include "yapl/i_media_extractor.hpp"
#include "yapl/i_media_source.hpp"
#include <memory>

namespace yapl {

/**
 * @brief Factory interface for creating media extractor instances.
 *
 * This interface follows the Factory pattern to create i_media_extractor objects.
 * Implementations can provide different extractor backends (e.g., FFmpeg-based,
 * platform-specific) while maintaining a consistent creation interface.
 */
struct i_media_extractor_factory {
    virtual ~i_media_extractor_factory() = default;

    /**
     * @brief Creates a new media extractor for the given media source.
     *
     * @param media_source The media source to extract data from
     * @return A unique pointer to the newly created media extractor
     * @throws std::runtime_error if the extractor cannot be created or the
     *         media source is incompatible
     */
    virtual std::unique_ptr<i_media_extractor> create(
        std::shared_ptr<i_media_source> media_source) = 0;
};

} // namespace yapl
