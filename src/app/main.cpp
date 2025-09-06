#include <cstdlib>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <zmq.hpp>
#include <vision_common/constants.hpp>
#include <vision_common/logging.hpp>
#include <tbb/concurrent_queue.h>

#include "config/app_config.hpp"
#include "services/camera_service.hpp"
#include "services/ai_service.hpp"
#include "services/control_service.hpp"

tbb::concurrent_queue<int> job_queue;

int main(int argc, char* argv[]) {
    VisionCommon::init_logging("/var/log/vision/backend.log");
    spdlog::cfg::load_env_levels();
    spdlog::info("Application started. Current log level: {}", spdlog::level::to_string_view(spdlog::get_level()));

    gst_init(&argc, &argv);

    CameraService camera;

    AiService ai(AppConfig::context(), job_queue, camera.get_inference_appsink());

    ControlService control(AppConfig::context(), camera);
    control.bind(VisionCommon::COMMAND_ENDPOINT);

    int counter = 0;
    while (true) {
        control.poll();
        //job_queue.push(counter++);
        //spdlog::info("[Main] pushed job {}", counter);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    spdlog::info("Application finished");
    return 0;
}
