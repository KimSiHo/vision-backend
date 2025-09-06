#pragma once

#include <optional>
#include <string>
#include <zmq.hpp>

class RepSocket {
public:
  RepSocket(zmq::context_t& ctx, const std::string_view endpoint);

  std::optional<std::string> receive();
  void send(const std::string_view msg);
  void* handle() { return static_cast<void*>(socket_); }

  RepSocket(const RepSocket&) = delete;
  RepSocket& operator=(const RepSocket&) = delete;
  RepSocket(RepSocket&) noexcept = default;
  RepSocket& operator=(RepSocket&) noexcept = default;

private:
  zmq::socket_t socket_;
};
