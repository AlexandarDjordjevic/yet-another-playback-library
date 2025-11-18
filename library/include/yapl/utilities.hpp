#pragma once

#include <functional>

namespace yapl::utilities {

struct raii_cleanup {
    explicit raii_cleanup(std::function<void()> cleanup_function)
        : m_cleanup{cleanup_function} {}
    ~raii_cleanup() { m_cleanup(); }

  private:
    std::function<void()> m_cleanup;
};

} // namespace yapl::utilities
