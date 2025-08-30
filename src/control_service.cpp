#include <thread>
#include <chrono>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "control_service.hpp"
#include "camera_service.hpp"
#include "ai_service.hpp"

using json = nlohmann::json;

namespace {
    enum class CommandType {
        Start,
        Stop,
        SwitchToCamera,
        SwitchToVideo,
        Unknown
    };

    CommandType resolveCommand(const std::string& cmd_str) {
        if (cmd_str == "START") return CommandType::Start;
        if (cmd_str == "STOP") return CommandType::Stop;
        if (cmd_str == "SWITCH_TO_CAMERA") return CommandType::SwitchToCamera;
        if (cmd_str == "SWITCH_TO_VIDEO") return CommandType::SwitchToVideo;

        return CommandType::Unknown;
    }
}

ControlService::ControlService(zmq::context_t& ctx,
                               CameraService& camera)
    : camera_(camera), rep_(ctx, zmq::socket_type::rep) { }

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

    switch (resolveCommand(cmd)) {
        case CommandType::Start:
            camera_.start();
//            ai_.start();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            reply = {{"ok", true}, {"msg", "started"}};
            break;

        case CommandType::Stop:
            camera_.stop();
//            ai_.stop();
            reply = {{"ok", true}, {"msg", "stopped"}};
            break;

        case CommandType::SwitchToCamera:
            camera_.switch_to_camera();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            reply = {{"ok", true}, {"msg", "switch to camera"}};
            break;

        case CommandType::SwitchToVideo:
            camera_.switch_to_uri();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            reply = {{"ok", true}, {"msg", "switch to video"}};
            break;

        default:
            reply = {{"ok", false}, {"msg", "unknown"}};
            break;
    }

    rep_.send(zmq::buffer(reply.dump()), zmq::send_flags::none);
}
