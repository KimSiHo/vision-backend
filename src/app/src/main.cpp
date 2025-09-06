#include <utility>

#include "common/utils/logging.hpp"
#include "common/zmq/pub_socket.hpp"
#include "common/zmq/rep_socket.hpp"
#include "config/app_config.hpp"
#include "config/zmq_config.hpp"
#include "services/audio/audio_service.hpp"
#include "services/bluetooth/bluetooth_service.hpp"
#include "services/camera/camera_service.hpp"
#include "services/control/control_service.hpp"
#include "services/infer/ai_service.hpp"

void initLogging() {
  app_common::initLogging(app_config::kLogFile);
  SPDLOG_INFO(R"(
===============================================
Application started.
Default logger level: {}
Service logger level: {}
ZMQ logger level: {}
===============================================
        )",
              spdlog::level::to_string_view(spdlog::default_logger()->level()),
              spdlog::level::to_string_view(spdlog::get("service")->level()),
              spdlog::level::to_string_view(spdlog::get("zmq")->level()));
}

int main(int argc, char* argv[]) {
  initLogging();
  gst_init(&argc, &argv);

  zmq::context_t ctx{1};
  PubSocket pub_socket(ctx, app_config::kEventEndpoint);
  RepSocket rep_socket(ctx, app_config::kControlEndpoint);

  // 음악 서비스
  MusicService music(MusicService::PipelineMode::Custom, pub_socket);

  // 카메라 서비스
  CameraService camera;

  // 블루투스 서비스
  BluetoothService bt(pub_socket);

  // 오디오 서비스
  AudioService audio;

  // 추론 서비스
  AiService ai(camera.getInferenceAppsink(), pub_socket);

  ControlService control(rep_socket);
  control.registerMusicService(music);
  control.registerCameraService(camera);
  control.registerBluetoothService(bt);
  control.registerAudioService(audio);

  control.poll();

  SPDLOG_INFO(R"(
===============================================
Application finished
===============================================
        )");

  return 0;
}
