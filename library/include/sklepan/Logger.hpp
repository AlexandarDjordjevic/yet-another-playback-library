#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

namespace sklepan {

class Logger {
public:
    static std::shared_ptr<spdlog::logger>& get() {
        static auto instance = [] {
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%# %!] %v");

            std::vector<spdlog::sink_ptr> sinks;
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
   
            auto _logger = std::make_shared<spdlog::logger>("app", begin(sinks), end(sinks));
            _logger->set_level(spdlog::level::trace);
            spdlog::register_logger(_logger);
            return _logger;
        }();
        return instance;
    }

private:
    static std::shared_ptr<spdlog::logger> _logger;
};


} // namespace sklepan


