#include <utility>

#include "services/camera/camera_service.hpp"
#include "services/infer/ai_service.hpp"
#include "services/bluetooth/bluetooth_service.hpp"
#include "services/audio/audio_service.hpp"
#include "services/control/control_service.hpp"
#include "common/utils/logging.hpp"
#include "common/zmq/pub_socket.hpp"
#include "common/zmq/rep_socket.hpp"
#include "config/app_config.hpp"
#include "config/zmq_config.hpp"

void init_logging() {
    AppCommon::initLogging(AppConfig::LOG_FILE);
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
        spdlog::level::to_string_view(spdlog::get("zmq")->level())
    );
}

int main(int argc, char* argv[]) {
    init_logging();
    gst_init(&argc, &argv);

    zmq::context_t ctx{1};
    PubSocket pubSocket(ctx, AppConfig::EVENT_ENDPOINT);
    RepSocket repSocket(ctx, AppConfig::CONTROL_ENDPOINT);

    // 음악 서비스
    MusicService music(MusicService::PipelineMode::Custom, pubSocket);

    // 카메라 서비스
    CameraService camera;

    // 블루투스 서비스
    BluetoothService bt(pubSocket);

    // 오디오 서비스
    AudioService audio;

    // 추론 서비스
    AiService ai(camera.get_inference_appsink(), pubSocket);

    ControlService control(repSocket);
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
