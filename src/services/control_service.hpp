#pragma once

#include <zmq.hpp>

class CameraService;
class AiService;

class ControlService {
public:
    ControlService(zmq::context_t& ctx, CameraService& camera);

    void bind(const std::string& endpoint);
    void poll();

private:
    CameraService& camera_;

    zmq::socket_t rep_;
};
