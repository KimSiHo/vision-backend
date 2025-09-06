#include "common/zmq/pub_socket.hpp"

#include "common/utils/logging.hpp"

PubSocket::PubSocket(zmq::context_t& ctx, const std::string_view endpoint) : socket_(ctx, zmq::socket_type::pub) {
  try {
    socket_.bind(std::string(endpoint));
    SPDLOG_ZMQ_INFO("PubSocket bind: {}", endpoint);
  } catch (const zmq::error_t& e) {
    SPDLOG_ZMQ_ERROR("PubSocket init failed: {}", e.what());
  }
}

void PubSocket::publish(const std::string_view topic, const std::string_view msg) {
  try {
    socket_.send(zmq::buffer(topic), zmq::send_flags::sndmore);
    socket_.send(zmq::buffer(msg), zmq::send_flags::none);
    SPDLOG_ZMQ_DEBUG("PubSocket published: topic={}, msg={}", topic, msg);
  } catch (const zmq::error_t& e) {
    SPDLOG_ZMQ_ERROR("PubSocket failed to publish: {}", e.what());
  }
}
