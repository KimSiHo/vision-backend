#pragma once

#include <gst/app/gstappsink.h>
#include <gst/gst.h>

#include <atomic>
#include <thread>

#include "common/zmq/pub_socket.hpp"

class AiService {
public:
  AiService(GstElement* appsink_elem, PubSocket& pub_socket);
  ~AiService();

  void start();
  void stop();

  AiService(const AiService&) = delete;
  AiService& operator=(const AiService&) = delete;

private:
  static GstFlowReturn onNewSample(GstAppSink* sink, gpointer user_data);
  void run();
  void attach(GstElement* appsink_elem);
  void detach();

  std::atomic<bool> running_{false};
  std::thread processing_thread_;
  GstAppSink* sink_{nullptr};
  PubSocket& pub_socket_;
};
