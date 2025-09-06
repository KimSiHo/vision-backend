#pragma once

#include <thread>
#include <atomic>

#include <tbb/concurrent_queue.h>
#include <vision_common/pub_socket.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class AiService {
public:
    AiService(zmq::context_t& ctx, tbb::concurrent_queue<int>& q, GstElement* appsink_elem);
    ~AiService();

    void start();
    void stop();

    AiService(const AiService&) = delete;
    AiService& operator=(const AiService&) = delete;

private:
    void run();
    static GstFlowReturn on_new_sample(GstAppSink* sink, gpointer user_data);

    void attach(GstElement* appsink_elem);
    void detach();

    std::atomic<bool> running_{false};
    std::thread processing_thread_;

    tbb::concurrent_queue<int>& job_queue_;
    PubSocket pub_;
    GstAppSink* sink_{nullptr};
};
