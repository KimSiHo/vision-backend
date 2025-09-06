#include "services/music/music_service.hpp"

#include <vector>
#include <string>

#include "common/utils/logging.hpp"
#include "common/utils/json.hpp"
#include "config/zmq_config.hpp"
#include "impl/music/playbin-pipeline/playbin_pipeline.hpp"
#include "impl/music/custom-pipeline/custom_pipeline.hpp"

namespace {
    struct TrackInfo {
        std::string path;
        std::string title;
        std::string cover_url;
    };

    std::vector<TrackInfo> playlist = {
        { "/opt/assets/track1.mp3", "Naughty Boy - Runnin' (Lose It All) ft. Beyoncé, Arrow Benjamin",
            "https://siho-docker-images.s3.ap-northeast-2.amazonaws.com/(Naughty+Boy+-+Runnin).jpg" },
        { "/opt/assets/track2.mp3", "Dusk Till Dawn",
            "https://siho-docker-images.s3.ap-northeast-2.amazonaws.com/(Dusk+Till+Dawn).jpg" },
        { "/opt/assets/track3.mp3", "I Don’t Wanna Live Forever",
            "https://siho-docker-images.s3.ap-northeast-2.amazonaws.com/(I+Don%E2%80%99t+Wanna+Live+Forever).jpg" }
    };

    int current_index = 0;
}

MusicService::MusicService(PipelineMode mode, PubSocket& pubSocket)
    : pipeline(nullptr),
      gst_loop(nullptr),
      pubSocket_(pubSocket) {
    SPDLOG_SERVICE_INFO("Music Using {} pipeline", (mode == PipelineMode::Playbin) ? "playbin" : "custom");

    gst_init(nullptr, nullptr);

    if (mode == PipelineMode::Playbin) {
        pipeline = new PlaybinPipeline();
    } else {
        pipeline = new CustomPipeline();
    }

    pipeline->set_uri(playlist[current_index].path);

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline->get_raw_pipeline()));
    gst_bus_add_watch(bus, busCallback, this);
    gst_object_unref(bus);

    // GMainLoop 시작 (별도 스레드)
    gst_loop = g_main_loop_new(nullptr, FALSE);
    gst_thread = std::thread(&MusicService::run_gst_loop, gst_loop);
}

MusicService::~MusicService() {
    if (gst_loop) {
        g_main_loop_quit(gst_loop);
    }
    if (gst_thread.joinable()) {
        gst_thread.join();
    }
    if (gst_loop) {
        g_main_loop_unref(gst_loop);
    }

    delete pipeline;
}

void MusicService::play() { pipeline->play(); }

void MusicService::pause() { pipeline->pause(); }

void MusicService::stop() { pipeline->stop(); }

void MusicService::next() {
    current_index = (current_index + 1) % playlist.size();
    pipeline->set_uri(playlist[current_index].path);
    play();
}

void MusicService::prev() {
    current_index = (current_index - 1 + playlist.size()) % playlist.size();
    pipeline->set_uri(playlist[current_index].path);
    play();
}

gboolean MusicService::busCallback(GstBus* bus, GstMessage* msg, gpointer user_data) {
    auto* self = static_cast<MusicService*>(user_data);

    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_STATE_CHANGED) {
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->get_pipeline()->get_raw_pipeline())) {
            GstState old_state, new_state;
            gst_message_parse_state_changed(msg, &old_state, &new_state, nullptr);

            if (new_state == GST_STATE_PLAYING) {
                const auto& track = playlist[current_index];

                AppCommon::json jmsg = {
                        { "title", track.title },
                        { "cover_url", track.cover_url }
                };

                self->pubSocket_.publish(AppConfig::TOPIC_TRACK_CHANGED, jmsg.dump());
            }
        }
    }

    return TRUE;
}

void MusicService::run_gst_loop(GMainLoop* loop) {
    SPDLOG_SERVICE_INFO("Starting GStreamer loop thread");
    g_main_loop_run(loop);
}
