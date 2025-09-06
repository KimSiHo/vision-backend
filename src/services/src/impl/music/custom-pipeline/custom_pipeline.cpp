#include "impl/music/custom-pipeline/custom_pipeline.hpp"

#include "common/utils/logging.hpp"
#include "impl/music/custom-pipeline/source_bin.hpp"
#include "impl/music/custom-pipeline/sink_bin.hpp"

CustomPipeline::CustomPipeline() {
    pipeline = gst_pipeline_new("custom-pipeline");
    if (!pipeline) {
        SPDLOG_SERVICE_ERROR("[Pipeline] Failed to create pipeline");
        return;
    }

    sourceBin = create_source_bin();
    sinkBin  = create_sink_bin();

    if (!sourceBin || !sinkBin) {
        SPDLOG_SERVICE_ERROR("[Pipeline] Failed to create bins");
        return;
    }

    gst_bin_add_many(GST_BIN(pipeline), sourceBin, sinkBin, nullptr);

    if (!gst_element_link_pads(sourceBin, "src", sinkBin, "sink")) {
        SPDLOG_SERVICE_ERROR("[Pipeline] Failed to link source-bin:src → sink-bin:sink");
    }
}

CustomPipeline::~CustomPipeline() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline   = nullptr;
        sourceBin  = nullptr;
        sinkBin   = nullptr;
    }
}

void CustomPipeline::set_uri(const std::string& uri) {
    if (!sourceBin) return;

    gst_element_set_state(pipeline, GST_STATE_NULL);  // 전체 파이프라인 정지

    GstElement* filesrc = gst_bin_get_by_name(GST_BIN(sourceBin), "file-source");
    if (!filesrc) {
        SPDLOG_SERVICE_ERROR("[Pipeline] filesrc not found inside sourceBin");
        return;
    }

    g_object_set(filesrc, "location", uri.c_str(), nullptr);
    gst_object_unref(filesrc);

    SPDLOG_SERVICE_INFO("[Pipeline] Set URI: {}", uri);
}

void CustomPipeline::play() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline),
                                  GST_DEBUG_GRAPH_SHOW_ALL,
                                  "music-custom-pipeline-playing");
        SPDLOG_SERVICE_INFO("[Pipeline] State -> PLAYING");
    }
}

void CustomPipeline::pause() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
        SPDLOG_SERVICE_INFO("[Pipeline] State -> PAUSED");
    }
}

void CustomPipeline::stop() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        SPDLOG_SERVICE_INFO("[Pipeline] State -> STOPPED");
    }
}
