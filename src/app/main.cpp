#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <vision_common/constants.hpp>
#include <vision_common/logging.hpp>

#include "config/app_config.hpp"
#include "services/camera_service.hpp"
#include "services/ai_service.hpp"
#include "services/bluetooth_service.hpp"
#include "services/control_service.hpp"
#include "services/audio_service.hpp"

int main(int argc, char* argv[]) {
    VisionCommon::init_logging("/var/log/vision/backend.log");
    spdlog::cfg::load_env_levels();
    spdlog::info("Application started. Current log level: {}", spdlog::level::to_string_view(spdlog::get_level()));

    gst_init(&argc, &argv);

    // music 서비스
    MusicService music(MusicService::PipelineMode::Custom);

    // 카메라 서비스
    CameraService camera;

    // 블루투스 서비스
    BluetoothService bluetooth;

    // 오디오 서비스
    AudioService audio;

    // ai 서비스
    AiService ai(AppConfig::context(), camera.get_inference_appsink());

    // 컨트롤 서비스
    ControlService control(AppConfig::context());
    control.bind(VisionCommon::COMMAND_ENDPOINT);

    control.registerMusicService(music);
    control.registerCameraService(camera);
    control.registerBluetoothService(bluetooth);
    control.registerAudioService(audio);

    control.poll();

    spdlog::info("Application finished");
    return 0;
}
