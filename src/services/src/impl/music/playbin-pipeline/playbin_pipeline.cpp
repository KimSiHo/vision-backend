#include "impl/music/playbin-pipeline/playbin_pipeline.hpp"

#include <stdexcept>

PlaybinPipeline::PlaybinPipeline() {
    pipeline = gst_element_factory_make("playbin", "player");
    if (!pipeline)
        throw std::runtime_error("Failed to create playbin pipeline");
}

PlaybinPipeline::~PlaybinPipeline() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
}

void PlaybinPipeline::set_uri(const std::string& path) {
    std::string uri = "file://" + path;
    g_object_set(pipeline, "uri", uri.c_str(), nullptr);
}

void PlaybinPipeline::play() {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    GstBus* bus = gst_element_get_bus(pipeline);
    GstMessage* msg = gst_bus_timed_pop_filtered(
        bus, GST_CLOCK_TIME_NONE,
        (GstMessageType)(GST_MESSAGE_ASYNC_DONE | GST_MESSAGE_ERROR));

    if (msg) {
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline),
                                  GST_DEBUG_GRAPH_SHOW_ALL,
                                  "music-pipeline-playing");
        gst_message_unref(msg);
    }

    gst_object_unref(bus);
}

void PlaybinPipeline::pause() {
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void PlaybinPipeline::stop() {
    gst_element_set_state(pipeline, GST_STATE_NULL);
}
