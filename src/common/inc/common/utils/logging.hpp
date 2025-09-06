#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <string>

#define SPDLOG_ZMQ_DEBUG(...) SPDLOG_LOGGER_DEBUG(app_common::getLogger("zmq"), __VA_ARGS__)
#define SPDLOG_ZMQ_INFO(...) SPDLOG_LOGGER_INFO(app_common::getLogger("zmq"), __VA_ARGS__)
#define SPDLOG_ZMQ_WARN(...) SPDLOG_LOGGER_WARN(app_common::getLogger("zmq"), __VA_ARGS__)
#define SPDLOG_ZMQ_ERROR(...) SPDLOG_LOGGER_ERROR(app_common::getLogger("zmq"), __VA_ARGS__)

#define SPDLOG_SERVICE_DEBUG(...) SPDLOG_LOGGER_DEBUG(app_common::getLogger("service"), __VA_ARGS__)
#define SPDLOG_SERVICE_INFO(...) SPDLOG_LOGGER_INFO(app_common::getLogger("service"), __VA_ARGS__)
#define SPDLOG_SERVICE_WARN(...) SPDLOG_LOGGER_WARN(app_common::getLogger("service"), __VA_ARGS__)
#define SPDLOG_SERVICE_ERROR(...) SPDLOG_LOGGER_ERROR(app_common::getLogger("service"), __VA_ARGS__)

namespace app_common {

inline std::shared_ptr<spdlog::logger> createLogger(const std::string_view name, const std::string_view path) {
  auto logger = spdlog::basic_logger_mt(std::string(name), std::string(path));
  logger->set_level(spdlog::level::info);
  logger->set_pattern("%H:%M:%S[%^%l%$][%n][%s:%!] %v");
  logger->flush_on(spdlog::level::info);
  return logger;
}

inline std::shared_ptr<spdlog::logger> getLogger(const std::string_view name) { return spdlog::get(std::string(name)); }

inline void initLogging(const std::string_view path) {
  auto default_logger = createLogger("main", path);
  auto service_logger = createLogger("service", path);
  auto zmq_logger = createLogger("zmq", path);

  default_logger->set_level(spdlog::level::info);
  service_logger->set_level(spdlog::level::info);
  zmq_logger->set_level(spdlog::level::info);

  spdlog::set_default_logger(default_logger);
}

}  // namespace app_common
