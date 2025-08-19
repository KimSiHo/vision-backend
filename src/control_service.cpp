#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "control_service.hpp"
#include "camera_service.hpp"
#include "ai_service.hpp"

using json = nlohmann::json;

ControlService::ControlService(zmq::context_t& ctx,
                               CameraService& camera,
                               AiService& ai)
    : camera_(camera), ai_(ai), rep_(ctx, zmq::socket_type::rep) {
}

void ControlService::bind(const std::string& endpoint) {
    rep_.bind(endpoint);
    spdlog::info("ControlService bound at {}", endpoint);
}

void ControlService::poll() {
    zmq::message_t req;
    if (!rep_.recv(req, zmq::recv_flags::dontwait)) {
        return;
    }

    std::string cmd(static_cast<char*>(req.data()), req.size());
    spdlog::info("Received cmd: {}", cmd);

    json reply;
    if (cmd == "START") {
        camera_.capture_on();
        ai_.start();
        reply = {{"ok", true}, {"msg", "started"}};
    } else if (cmd == "STOP") {
        camera_.capture_off();
        ai_.stop();
        reply = {{"ok", true}, {"msg", "stopped"}};
    } else {
        reply = {{"ok", false}, {"msg", "unknown"}};
    }

    rep_.send(zmq::buffer(reply.dump()), zmq::send_flags::none);
}
