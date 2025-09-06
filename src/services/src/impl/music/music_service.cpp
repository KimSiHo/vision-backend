#include "services/music/music_service.hpp"

#include <string>
#include <vector>

#include "common/utils/json.hpp"
#include "common/utils/logging.hpp"
#include "config/zmq_config.hpp"
#include "impl/music/custom-pipeline/custom_pipeline.hpp"
#include "impl/music/playbin-pipeline/playbin_pipeline.hpp"

namespace {
struct TrackInfo {
  std::string path;
  std::string title;
  std::string cover_url;
};

std::vector<TrackInfo> kPlaylist = {
    {"/opt/assets/track1.mp3", "Naughty Boy - Runnin' (Lose It All) ft. Beyoncé, Arrow Benjamin",
     "https://siho-docker-images.s3.ap-northeast-2.amazonaws.com/(Naughty+Boy+-+Runnin).jpg"},
    {"/opt/assets/track2.mp3", "Dusk Till Dawn",
     "https://siho-docker-images.s3.ap-northeast-2.amazonaws.com/(Dusk+Till+Dawn).jpg"},
    {"/opt/assets/track3.mp3", "I Don’t Wanna Live Forever",
     "https://siho-docker-images.s3.ap-northeast-2.amazonaws.com/(I+Don%E2%80%99t+Wanna+Live+Forever).jpg"}};

int current_index = 0;
}  // namespace

MusicService::MusicService(PipelineMode mode, PubSocket& pub_socket)
    : pipeline_(nullptr), gst_loop_(nullptr), pub_socket_(pub_socket) {
  SPDLOG_SERVICE_INFO("Music Using {} pipeline", (mode == PipelineMode::Playbin) ? "playbin" : "custom");

  gst_init(nullptr, nullptr);

  if (mode == PipelineMode::Playbin) {
    pipeline_ = new PlaybinPipeline();
  } else {
    pipeline_ = new CustomPipeline();
  }

  pipeline_->setUri(kPlaylist[current_index].path);

  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_->getRawPipeline()));
  gst_bus_add_watch(bus, busCallback, this);
  gst_object_unref(bus);

  gst_loop_ = g_main_loop_new(nullptr, FALSE);
  gst_thread_ = std::thread(&MusicService::runGstLoop, gst_loop_);
}

MusicService::~MusicService() {
  if (gst_loop_) {
    g_main_loop_quit(gst_loop_);
  }
  if (gst_thread_.joinable()) {
    gst_thread_.join();
  }
  if (gst_loop_) {
    g_main_loop_unref(gst_loop_);
  }

  delete pipeline_;
}

void MusicService::play() { pipeline_->play(); }

void MusicService::pause() { pipeline_->pause(); }

void MusicService::stop() { pipeline_->stop(); }

void MusicService::next() {
  current_index = (current_index + 1) % kPlaylist.size();
  pipeline_->setUri(kPlaylist[current_index].path);
  play();
}

void MusicService::prev() {
  current_index = (current_index - 1 + kPlaylist.size()) % kPlaylist.size();
  pipeline_->setUri(kPlaylist[current_index].path);
  play();
}

gboolean MusicService::busCallback(GstBus* bus, GstMessage* msg, gpointer user_data) {
  auto* self = static_cast<MusicService*>(user_data);

  if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_STATE_CHANGED) {
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->getPipeline()->getRawPipeline())) {
      GstState old_state, new_state;
      gst_message_parse_state_changed(msg, &old_state, &new_state, nullptr);

      if (new_state == GST_STATE_PLAYING) {
        const auto& track = kPlaylist[current_index];

        app_common::Json jmsg = {{"title", track.title}, {"cover_url", track.cover_url}};

        self->pub_socket_.publish(app_config::kTopicTrackChanged, jmsg.dump());
      }
    }
  }

  return TRUE;
}

void MusicService::runGstLoop(GMainLoop* loop) {
  SPDLOG_SERVICE_INFO("Starting GStreamer loop thread");
  g_main_loop_run(loop);
}
