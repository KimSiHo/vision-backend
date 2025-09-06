#include "adapters/camera/camera_service_adapter.hpp"

CameraServiceAdapter::CameraServiceAdapter(CameraService& service) : service_(service) {}

bool CameraServiceAdapter::handle(const std::string& command, app_common::Json& reply) {
  if (command == "CAMERA_START") {
    service_.start();
    reply = {{"ok", true}, {"msg", "camera started"}};
    return true;
  }

  if (command == "CAMERA_STOP") {
    service_.stop();
    reply = {{"ok", true}, {"msg", "camera stopped"}};
    return true;
  }

  if (command == "SWITCH_TO_CAMERA") {
    service_.switchToCamera();
    reply = {{"ok", true}, {"msg", "switched to camera"}};
    return true;
  }

  if (command == "SWITCH_TO_TEST") {
    service_.switchToTest();
    reply = {{"ok", true}, {"msg", "switched to test"}};
    return true;
  }

  return false;
}
