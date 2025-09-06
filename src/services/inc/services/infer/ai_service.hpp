#pragma once

#include <thread>
#include <atomic>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include "common/zmq/pub_socket.hpp"

class AiService {
public:
    AiService(GstElement* appsink_elem, PubSocket& pubSocket);
    ~AiService();

    void start();
    void stop();

    AiService(const AiService&) = delete;
    AiService& operator=(const AiService&) = delete;

private:
    static GstFlowReturn on_new_sample(GstAppSink* sink, gpointer user_data);
    void run();
    void attach(GstElement* appsink_elem);
    void detach();

    std::atomic<bool> running_{false};
    std::thread processing_thread_;
    GstAppSink* sink_{nullptr};
    PubSocket& pubSocket_;
};
