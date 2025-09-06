#pragma once

#include <gst/gst.h>

#include <atomic>
#include <thread>

class CameraService {
public:
  CameraService();
  ~CameraService();

  void start();
  void stop();
  void switchToCamera();
  void switchToTest();
  GstElement* getInferenceAppsink() { return inference_appsink_; }

private:
  GstElement* buildPipeline();
  bool createElements();
  void configureElements();
  bool linkElements();
  void installPadProbe();

  void busWatchFunction();

  static void onPadAdded(GstElement* src, GstPad* new_pad, gpointer user_data);
  static gboolean onAutoplugContinue(GstElement* bin, GstPad* pad, GstCaps* caps, gpointer user_data);
  void linkUridecodebinPad(GstPad* new_pad);

  GstElement* pipeline_{nullptr};
  GstElement* camera_src_{nullptr};
  GstElement* camera_caps_nvmm_{nullptr};
  GstElement* camera_conv_{nullptr};
  GstElement* camera_caps_scaled_{nullptr};
  GstElement* uri_src_{nullptr};
  GstElement* uri_queue_{nullptr};
  GstElement* uri_conv_{nullptr};
  GstElement* uri_caps_scaled_{nullptr};
  GstElement* uri_conv2_{nullptr};
  GstElement* uri_caps_scaled2_{nullptr};
  GstElement* audio_fakesink_{nullptr};
  GstElement* src_selector_{nullptr};
  GstElement* tee_{nullptr};
  GstElement* front_queue_{nullptr};
  GstElement* front_conv_{nullptr};
  GstElement* front_caps_{nullptr};
  GstElement* uri_videorate_{nullptr};
  GstElement* uri_caps_framerate_{nullptr};
  GstElement* front_shm_{nullptr};
  GstElement* inference_queue_{nullptr};
  GstElement* inference_conv_{nullptr};
  GstElement* inference_caps_nvmm_{nullptr};
  GstElement* inference_streammux_{nullptr};
  GstElement* inference_nvinfer_{nullptr};
  GstElement* inference_conv3_{nullptr};
  GstElement* inference_caps_sys_{nullptr};
  GstElement* inference_appsink_{nullptr};

  GstBus* bus_{nullptr};
  std::thread bus_thread_;
  std::atomic<bool> is_active_{false};
};
