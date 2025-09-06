#include "services/control_service.hpp"

#include <spdlog/spdlog.h>

#include "common/json.hpp"
#include "adapters/i_service.hpp"
#include "adapters/music_service_adapter.hpp"
#include "adapters/camera_service_adapter.hpp"
#include "adapters/bluetooth_service_adapter.hpp"
#include "adapters/audio_service_adapter.hpp"

ControlService::ControlService(zmq::context_t& ctx)
    : rep_(ctx, zmq::socket_type::rep) {}

void ControlService::registerMusicService(MusicService& svc) {
    services_.push_back(new MusicServiceAdapter(svc));
}

void ControlService::registerCameraService(CameraService& svc) {
    services_.push_back(new CameraServiceAdapter(svc));
}

void ControlService::registerBluetoothService(BluetoothService& svc) {
    services_.push_back(new BluetoothServiceAdapter(svc));
}

void ControlService::registerAudioService(AudioService& svc) {
    services_.push_back(new AudioServiceAdapter(svc));
}

void ControlService::bind(const std::string& endpoint) {
    rep_.bind(endpoint);
    spdlog::info("ControlService bound at {}", endpoint);
}

void ControlService::poll() {
    zmq::pollitem_t items[] = {
        { static_cast<void*>(rep_), 0, ZMQ_POLLIN, 0 }
    };

    while (true) {
        zmq::poll(items, 1, -1);

        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t req;
            rep_.recv(req, zmq::recv_flags::none);

            std::string cmd(static_cast<char*>(req.data()), req.size());
            spdlog::info("Received cmd: {}", cmd);

            common::json reply;
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

            rep_.send(zmq::buffer(reply.dump()), zmq::send_flags::none);
        }
    }
}
