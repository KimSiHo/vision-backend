#include "impl/music/custom-pipeline/custom_pipeline.hpp"

#include "common/utils/logging.hpp"
#include "impl/music/custom-pipeline/sink_bin.hpp"
#include "impl/music/custom-pipeline/source_bin.hpp"

CustomPipeline::CustomPipeline() {
  pipeline_ = gst_pipeline_new("custom-pipeline");
  if (!pipeline_) {
    SPDLOG_SERVICE_ERROR("[Pipeline] Failed to create pipeline");
    return;
  }

  source_bin_ = createSourceBin();
  sink_bin_ = createSinkBin();

  if (!source_bin_ || !sink_bin_) {
    SPDLOG_SERVICE_ERROR("[Pipeline] Failed to create bins");
    return;
  }

  gst_bin_add_many(GST_BIN(pipeline_), source_bin_, sink_bin_, nullptr);

  if (!gst_element_link_pads(source_bin_, "src", sink_bin_, "sink")) {
    SPDLOG_SERVICE_ERROR("[Pipeline] Failed to link source-bin:src → sink-bin:sink");
  }
}

CustomPipeline::~CustomPipeline() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;
    source_bin_ = nullptr;
    sink_bin_ = nullptr;
  }
}

void CustomPipeline::setUri(const std::string& uri) {
  if (!source_bin_) return;

  gst_element_set_state(pipeline_, GST_STATE_NULL);  // 전체 파이프라인 정지

  GstElement* filesrc = gst_bin_get_by_name(GST_BIN(source_bin_), "file-source");
  if (!filesrc) {
    SPDLOG_SERVICE_ERROR("[Pipeline] filesrc not found inside source_bin_");
    return;
  }

  g_object_set(filesrc, "location", uri.c_str(), nullptr);
  gst_object_unref(filesrc);

  SPDLOG_SERVICE_INFO("[Pipeline] Set URI: {}", uri);
}

void CustomPipeline::play() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline_), GST_DEBUG_GRAPH_SHOW_ALL, "music-custom-pipeline-playing");
    SPDLOG_SERVICE_INFO("[Pipeline] State -> PLAYING");
  }
}

void CustomPipeline::pause() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_PAUSED);
    SPDLOG_SERVICE_INFO("[Pipeline] State -> PAUSED");
  }
}

void CustomPipeline::stop() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    SPDLOG_SERVICE_INFO("[Pipeline] State -> STOPPED");
  }
}
