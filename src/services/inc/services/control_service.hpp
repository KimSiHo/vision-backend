#pragma once

#include <string>

#include <zmq.hpp>

#include "services/music_service.hpp"
#include "services/camera_service.hpp"
#include "services/bluetooth_service.hpp"
#include "services/audio_service.hpp"

class ControlService {
public:
    explicit ControlService(zmq::context_t& ctx);

    void registerMusicService(MusicService& svc);
    void registerCameraService(CameraService& svc);
    void registerBluetoothService(BluetoothService& svc);
    void registerAudioService(AudioService& svc);

    void bind(const std::string& endpoint);
    void poll();

private:
    std::vector<class IService*> services_;
    zmq::socket_t rep_;
};
