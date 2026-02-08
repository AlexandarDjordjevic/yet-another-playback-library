#pragma once

#include "yapl/i_media_source.hpp"
#include <memory>

namespace yapl {

/**
 * @brief Factory interface for creating media source instances.
 *
 * This interface follows the Factory pattern to create i_media_source objects.
 * Implementations can provide different types of media sources (e.g., file-based,
 * network-based, memory-based) while maintaining a consistent creation interface.
 */
struct i_media_source_factory {
    virtual ~i_media_source_factory() = default;

    /**
     * @brief Creates a new media source instance.
     *
     * @return A shared pointer to the newly created media source
     * @throws std::runtime_error if the media source cannot be created
     */
    virtual std::shared_ptr<i_media_source> create() = 0;
};

} // namespace yapl
