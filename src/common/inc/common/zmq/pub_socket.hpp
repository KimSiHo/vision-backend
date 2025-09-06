#pragma once

#include <string>
#include <zmq.hpp>

class PubSocket {
public:
  PubSocket(zmq::context_t& ctx, const std::string_view endpoint);

  void publish(const std::string_view topic, const std::string_view msg);

  PubSocket(const PubSocket&) = delete;
  PubSocket& operator=(const PubSocket&) = delete;
  PubSocket(PubSocket&&) noexcept = default;
  PubSocket& operator=(PubSocket&&) noexcept = default;

private:
  zmq::socket_t socket_;
};
