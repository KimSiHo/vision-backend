#pragma once

#include <gst/gst.h>

#include <string>
#include <thread>

#include "common/zmq/pub_socket.hpp"
#include "services/music/pipeline_wrapper.hpp"

class MusicService {
public:
  enum class PipelineMode { Custom, Playbin };

  explicit MusicService(PipelineMode mode, PubSocket& pub_socket);
  ~MusicService();

  void play();
  void stop();
  void pause();
  void next();
  void prev();

  PipelineWrapper* getPipeline() const { return pipeline_; }

private:
  static gboolean busCallback(GstBus* bus, GstMessage* message, gpointer user_data);
  static void runGstLoop(GMainLoop* loop);

  PipelineWrapper* pipeline_{nullptr};
  GMainLoop* gst_loop_{nullptr};
  std::thread gst_thread_;
  PubSocket& pub_socket_;
};
