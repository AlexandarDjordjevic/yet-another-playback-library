#pragma once

#include "yapl/detail/logger.hpp"

// Helper macro for short filename
#define YAPL_FILE yapl::filename_only(__FILE__)

// Compile-time log stripping for TRACE/DEBUG in release builds
#ifdef NDEBUG

#define LOG_TRACE(...) (void)0
#define LOG_DEBUG(...) (void)0

#else

#define LOG_TRACE(...)                                                         \
    yapl::logger::get().log(spdlog::source_loc{YAPL_FILE, __LINE__, __func__}, \
                            spdlog::level::trace, __VA_ARGS__)

#define LOG_DEBUG(...)                                                         \
    yapl::logger::get().log(spdlog::source_loc{YAPL_FILE, __LINE__, __func__}, \
                            spdlog::level::debug, __VA_ARGS__)

#endif

#define LOG_INFO(...)                                                          \
    yapl::logger::get().log(spdlog::source_loc{YAPL_FILE, __LINE__, __func__}, \
                            spdlog::level::info, __VA_ARGS__)

#define LOG_WARN(...)                                                          \
    yapl::logger::get().log(spdlog::source_loc{YAPL_FILE, __LINE__, __func__}, \
                            spdlog::level::warn, __VA_ARGS__)

#define LOG_ERROR(...)                                                         \
    yapl::logger::get().log(spdlog::source_loc{YAPL_FILE, __LINE__, __func__}, \
                            spdlog::level::err, __VA_ARGS__)

#define LOG_CRITICAL(...)                                                      \
    do {                                                                       \
        yapl::logger::get().log(                                               \
            spdlog::source_loc{YAPL_FILE, __LINE__, __func__},                 \
            spdlog::level::critical, __VA_ARGS__);                             \
        yapl::logger::get().flush();                                           \
    } while (0)
