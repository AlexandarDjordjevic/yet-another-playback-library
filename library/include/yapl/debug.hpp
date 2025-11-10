#pragma once

#include "yapl/logger.hpp"

#define LOG_TRACE(...)                                                         \
    yapl::logger::get()->log(                                                  \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},               \
        spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...)                                                         \
    yapl::logger::get()->log(                                                  \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},               \
        spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)                                                          \
    yapl::logger::get()->log(                                                  \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},               \
        spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...)                                                          \
    yapl::logger::get()->log(                                                  \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},               \
        spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...)                                                         \
    yapl::logger::get()->log(                                                  \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},               \
        spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(...)                                                      \
    yapl::logger::get()->log(                                                  \
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},               \
        spdlog::level::critical, __VA_ARGS__)
