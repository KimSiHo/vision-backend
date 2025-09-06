#include "impl/music/custom-pipeline/custom_pipeline.hpp"

#include <spdlog/spdlog.h>

#include "impl/music/custom-pipeline/bin_source.hpp"
#include "impl/music/custom-pipeline/bin_audio.hpp"

CustomPipeline::CustomPipeline() {
    pipeline = gst_pipeline_new("custom-pipeline");
    if (!pipeline) {
        spdlog::error("[Pipeline] Failed to create pipeline");
        return;
    }

    sourceBin = create_source_bin();
    audioBin  = create_audio_bin();

    if (!sourceBin || !audioBin) {
        spdlog::error("[Pipeline] Failed to create bins");
        return;
    }

    gst_bin_add_many(GST_BIN(pipeline), sourceBin, audioBin, nullptr);

    if (!gst_element_link_pads(sourceBin, "src", audioBin, "sink")) {
        spdlog::error("[Pipeline] Failed to link source-bin:src → audio-bin:sink");
    }
}

CustomPipeline::~CustomPipeline() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline   = nullptr;
        sourceBin  = nullptr;
        audioBin   = nullptr;
    }
}

void CustomPipeline::set_uri(const std::string& uri) {
    if (!sourceBin) return;

    GstElement* filesrc = gst_bin_get_by_name(GST_BIN(sourceBin), "file-source");
    if (!filesrc) {
        spdlog::error("[Pipeline] filesrc not found inside sourceBin");
        return;
    }

    g_object_set(filesrc, "location", uri.c_str(), nullptr);
    gst_object_unref(filesrc);

    spdlog::info("[Pipeline] Set URI: {}", uri);
}

void CustomPipeline::play() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline),
                                  GST_DEBUG_GRAPH_SHOW_ALL,
                                  "music-custom-pipeline-playing");
        spdlog::info("[Pipeline] State -> PLAYING");
    }
}

void CustomPipeline::pause() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
        spdlog::info("[Pipeline] State -> PAUSED");
    }
}

void CustomPipeline::stop() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        spdlog::info("[Pipeline] State -> STOPPED");
    }
}
