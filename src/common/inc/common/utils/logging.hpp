#pragma once

#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#define SPDLOG_ZMQ_DEBUG(...) SPDLOG_LOGGER_DEBUG(AppCommon::getLogger("zmq"), __VA_ARGS__)
#define SPDLOG_ZMQ_INFO(...) SPDLOG_LOGGER_INFO(AppCommon::getLogger("zmq"), __VA_ARGS__)
#define SPDLOG_ZMQ_WARN(...) SPDLOG_LOGGER_WARN(AppCommon::getLogger("zmq"), __VA_ARGS__)
#define SPDLOG_ZMQ_ERROR(...) SPDLOG_LOGGER_ERROR(AppCommon::getLogger("zmq"), __VA_ARGS__)

#define SPDLOG_SERVICE_DEBUG(...) SPDLOG_LOGGER_DEBUG(AppCommon::getLogger("service"), __VA_ARGS__)
#define SPDLOG_SERVICE_INFO(...) SPDLOG_LOGGER_INFO(AppCommon::getLogger("service"), __VA_ARGS__)
#define SPDLOG_SERVICE_WARN(...) SPDLOG_LOGGER_WARN(AppCommon::getLogger("service"), __VA_ARGS__)
#define SPDLOG_SERVICE_ERROR(...) SPDLOG_LOGGER_ERROR(AppCommon::getLogger("service"), __VA_ARGS__)

namespace AppCommon {
    inline std::shared_ptr<spdlog::logger> createLogger(const std::string_view name, const std::string_view path) {
        auto logger = spdlog::basic_logger_mt(std::string(name), std::string(path));
        logger->set_level(spdlog::level::info);
        logger->set_pattern("%H:%M:%S[%^%l%$][%n][%s:%!] %v");
        logger->flush_on(spdlog::level::info);
        return logger;
    }

    inline std::shared_ptr<spdlog::logger> getLogger(const std::string_view name) {
        return spdlog::get(std::string(name));
    }

    inline void initLogging(const std::string_view path) {
        auto defaultLogger = createLogger("main", path);
        auto serviceLogger = createLogger("service", path);
        auto zmqLogger = createLogger("zmq", path);

        defaultLogger->set_level(spdlog::level::info);
        serviceLogger->set_level(spdlog::level::info);
        zmqLogger->set_level(spdlog::level::info);

        spdlog::set_default_logger(defaultLogger);
    }
}
