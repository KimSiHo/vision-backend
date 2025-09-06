#include "adapters/camera/camera_service_adapter.hpp"

CameraServiceAdapter::CameraServiceAdapter(CameraService& svc)
    : svc_(svc) {}

bool CameraServiceAdapter::handle(const std::string& cmd, nlohmann::json& reply) {
    if (cmd == "CAMERA_START") {
        svc_.start();
        reply = {{"ok", true}, {"msg", "camera started"}};
        return true;
    }
    if (cmd == "CAMERA_STOP") {
        svc_.stop();
        reply = {{"ok", true}, {"msg", "camera stopped"}};
        return true;
    }
    if (cmd == "SWITCH_TO_CAMERA") {
        svc_.switch_to_camera();
        reply = {{"ok", true}, {"msg", "switched to camera"}};
        return true;
    }
    if (cmd == "SWITCH_TO_TEST") {
        svc_.switch_to_test();
        reply = {{"ok", true}, {"msg", "switched to test"}};
        return true;
    }
    return false;
}
