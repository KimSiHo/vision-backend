#pragma once

#include <string>
#include <thread>
#include <gst/gst.h>

#include "services/music/pipeline_wrapper.hpp"
#include "common/zmq/pub_socket.hpp"

class MusicService {
public:
    enum class PipelineMode {
        Custom,
        Playbin
    };

    explicit MusicService(PipelineMode mode, PubSocket& pubSocket);
    ~MusicService();

    void play();
    void stop();
    void pause();
    void next();
    void prev();

    PipelineWrapper* get_pipeline() const { return pipeline; }

private:
    // callback functions
    static gboolean busCallback(GstBus* bus, GstMessage* msg, gpointer user_data);

    static void run_gst_loop(GMainLoop* loop);

    PipelineWrapper* pipeline;
    GMainLoop* gst_loop;
    std::thread gst_thread;
    PubSocket& pubSocket_;
};
