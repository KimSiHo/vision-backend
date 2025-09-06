#include "impl/music/playbin-pipeline/playbin_pipeline.hpp"

#include <stdexcept>

#include "common/utils/logging.hpp"

PlaybinPipeline::PlaybinPipeline() {
  pipeline_ = gst_element_factory_make("playbin", "player");
  if (!pipeline_) throw std::runtime_error("Failed to create playbin pipeline");
}

PlaybinPipeline::~PlaybinPipeline() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
  }
}

void PlaybinPipeline::setUri(const std::string& path) {
  std::string uri = "file://" + path;
  g_object_set(pipeline_, "uri", uri.c_str(), nullptr);
}

void PlaybinPipeline::play() {
  gst_element_set_state(pipeline_, GST_STATE_PLAYING);

  GstBus* bus = gst_element_get_bus(pipeline_);
  GstMessage* msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                               (GstMessageType)(GST_MESSAGE_ASYNC_DONE | GST_MESSAGE_ERROR));

  if (msg) {
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline_), GST_DEBUG_GRAPH_SHOW_ALL, "music-pipeline-playing");
    gst_message_unref(msg);
  }

  gst_object_unref(bus);
}

void PlaybinPipeline::pause() { gst_element_set_state(pipeline_, GST_STATE_PAUSED); }

void PlaybinPipeline::stop() { gst_element_set_state(pipeline_, GST_STATE_NULL); }
