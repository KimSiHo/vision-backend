#include <cstdlib>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <zmq.hpp>
#include <vision_common/constants.hpp>
#include <vision_common/logging.hpp>
#include <tbb/concurrent_queue.h>
#include <spdlog/cfg/env.h>  // load_env_levels

#include "app_config.hpp"
#include "camera_service.hpp"
#include "ai_service.hpp"
#include "control_service.hpp"

tbb::concurrent_queue<int> job_queue;

int main(int argc, char* argv[]) {
    VisionCommon::init_logging("/var/log/vision/backend.log");
    spdlog::cfg::load_env_levels();  // 여기서 SPDLOG_LEVEL 읽어서 적용

    spdlog::info("logger='{}' level={}",
                spdlog::default_logger()->name(),
                spdlog::level::to_string_view(spdlog::default_logger()->level()));

    gst_init(&argc, &argv);

    CameraService camera;

    //AiService ai(AppConfig::context(), job_queue, camera.get_inference_appsink());

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


//  // I/O 루프(메인)
//  while (running) {
//    io.Tick();
//    Frame f;
//    if (io.TryGetFrame(f)) q.push(std::move(f));
//
//    // 큐 소비는 워커들이:
//    pool.try_dispatch([&]{
//      if (auto f = q.pop()) {
//        auto r = infer.Run(*f);
//        io.Publish(r);
//      }
//    });
//  }