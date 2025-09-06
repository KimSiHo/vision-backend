#pragma once

#include <string>

#include "common/zmq/rep_socket.hpp"
#include "services/audio/audio_service.hpp"
#include "services/bluetooth/bluetooth_service.hpp"
#include "services/camera/camera_service.hpp"
#include "services/music/music_service.hpp"

class ControlService {
public:
  ControlService(RepSocket& rep_socket);

  void registerMusicService(MusicService& service);
  void registerCameraService(CameraService& service);
  void registerBluetoothService(BluetoothService& service);
  void registerAudioService(AudioService& service);
  void poll();

private:
  std::vector<class IService*> services_;
  RepSocket& rep_socket_;
};
