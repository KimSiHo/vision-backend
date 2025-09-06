#include "services/control/control_service.hpp"

#include "adapters/audio/audio_service_adapter.hpp"
#include "adapters/bluetooth/bluetooth_service_adapter.hpp"
#include "adapters/camera/camera_service_adapter.hpp"
#include "adapters/i_service.hpp"
#include "adapters/music/music_service_adapter.hpp"
#include "common/utils/json.hpp"
#include "common/utils/logging.hpp"
#include "config/zmq_config.hpp"

ControlService::ControlService(RepSocket& rep_socket) : rep_socket_(rep_socket) {}

void ControlService::registerMusicService(MusicService& svc) { services_.push_back(new MusicServiceAdapter(svc)); }

void ControlService::registerCameraService(CameraService& svc) { services_.push_back(new CameraServiceAdapter(svc)); }

void ControlService::registerBluetoothService(BluetoothService& svc) {
  services_.push_back(new BluetoothServiceAdapter(svc));
}

void ControlService::registerAudioService(AudioService& svc) { services_.push_back(new AudioServiceAdapter(svc)); }

void ControlService::poll() {
  zmq::pollitem_t items[] = {{rep_socket_.handle(), 0, ZMQ_POLLIN, 0}};

  while (true) {
    zmq::poll(items, 1, -1);

    if (items[0].revents & ZMQ_POLLIN) {
      auto cmd = rep_socket_.receive().value_or("");
      if (!cmd.empty()) {
        SPDLOG_SERVICE_INFO("Received cmd: {}", cmd);
      }

      app_common::Json reply;
      bool handled = false;

      for (auto* svc : services_) {
        if (svc->handle(cmd, reply)) {
          handled = true;
          break;
        }
      }

      if (!handled) {
        reply = {{"ok", false}, {"msg", "unknown command"}};
      }

      rep_socket_.send(reply.dump());
    }
  }
}
