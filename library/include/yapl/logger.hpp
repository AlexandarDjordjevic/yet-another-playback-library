#pragma once

#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace yapl {

class logger {
  public:
    static std::shared_ptr<spdlog::logger> &get() {
        static auto instance = [] {
            spdlog::set_pattern(
                "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%# %!] %v");

            std::vector<spdlog::sink_ptr> sinks;
            sinks.push_back(
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

            auto m_logger = std::make_shared<spdlog::logger>(
                "app", begin(sinks), end(sinks));
            m_logger->set_level(spdlog::level::trace);
            spdlog::register_logger(m_logger);
            return m_logger;
        }();
        return instance;
    }

  private:
    static std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace yapl
