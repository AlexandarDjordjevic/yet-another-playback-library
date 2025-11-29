#pragma once

#include <cstdlib>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string_view>
#include <vector>

namespace yapl {

constexpr const char *filename_only(const char *path) {
    const char *file = path;
    while (*path) {
        if (*path == '/' || *path == '\\') {
            file = path + 1;
        }
        ++path;
    }
    return file;
}

class logger {
  public:
    static spdlog::logger &get() {
        static auto instance = [] {
            std::vector<spdlog::sink_ptr> sinks;

            sinks.push_back(
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

            if (const char *log_file = std::getenv("YAPL_LOG_FILE")) {
                sinks.push_back(
                    std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                        log_file, true));
            }

            auto log = std::make_shared<spdlog::logger>("yapl", sinks.begin(),
                                                        sinks.end());

            // Pattern: [TIME][THREAD][LEVEL][file:line func] message
            log->set_pattern("[%H:%M:%S.%e][%t][%^%l%$][%s:%# %!] %v");
            log->set_level(level_from_env());

            spdlog::register_logger(log);
            return log;
        }();
        return *instance;
    }

    logger() = delete;

  private:
    static spdlog::level::level_enum level_from_env() {
        const char *env = std::getenv("YAPL_LOG_LEVEL");
        if (!env) {
            return spdlog::level::info; // Default level
        }

        return parse_level(env);
    }

    static spdlog::level::level_enum parse_level(std::string_view level) {
        auto to_lower = [](std::string_view s) {
            std::string result;
            result.reserve(s.size());
            for (char c : s) {
                result += static_cast<char>(
                    std::tolower(static_cast<unsigned char>(c)));
            }
            return result;
        };

        const auto lower = to_lower(level);

        if (lower == "trace")
            return spdlog::level::trace;
        if (lower == "debug")
            return spdlog::level::debug;
        if (lower == "info")
            return spdlog::level::info;
        if (lower == "warn" || lower == "warning")
            return spdlog::level::warn;
        if (lower == "error" || lower == "err")
            return spdlog::level::err;
        if (lower == "critical")
            return spdlog::level::critical;
        if (lower == "off")
            return spdlog::level::off;

        return spdlog::level::info; // Default for invalid values
    }
};

} // namespace yapl
