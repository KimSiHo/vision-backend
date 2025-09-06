#include "common/zmq/rep_socket.hpp"

#include "common/utils/logging.hpp"

RepSocket::RepSocket(zmq::context_t& ctx, const std::string_view endpoint) : socket_(ctx, zmq::socket_type::rep) {
  try {
    socket_.bind(std::string(endpoint));
    SPDLOG_ZMQ_INFO("RepSocket bind: {}", endpoint);
  } catch (const zmq::error_t& e) {
    SPDLOG_ZMQ_ERROR("RepSocket init failed: {}", e.what());
  }
}

std::optional<std::string> RepSocket::receive() {
  try {
    zmq::message_t request;
    if (socket_.recv(request, zmq::recv_flags::none)) {
      return request.to_string();
    }
  } catch (const zmq::error_t& e) {
    SPDLOG_ZMQ_ERROR("RepSocket receive error: {}", e.what());
  }
  return std::nullopt;
}

void RepSocket::send(const std::string_view msg) {
  try {
    socket_.send(zmq::buffer(msg), zmq::send_flags::none);
    SPDLOG_ZMQ_DEBUG("RepSocket sent reply: {}", msg);
  } catch (const zmq::error_t& e) {
    SPDLOG_ZMQ_ERROR("RepSocket failed to send reply: {}", e.what());
  }
}
