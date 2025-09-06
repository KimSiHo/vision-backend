#pragma once

#include <string>

#include "services/music/music_service.hpp"
#include "services/camera/camera_service.hpp"
#include "services/bluetooth/bluetooth_service.hpp"
#include "services/audio/audio_service.hpp"
#include "common/zmq/rep_socket.hpp"

class ControlService {
public:
    ControlService(RepSocket& repSocket);

    void registerMusicService(MusicService& svc);
    void registerCameraService(CameraService& svc);
    void registerBluetoothService(BluetoothService& svc);
    void registerAudioService(AudioService& svc);
    void poll();

private:
    std::vector<class IService*> services_;
    RepSocket& repSocket_;
};
