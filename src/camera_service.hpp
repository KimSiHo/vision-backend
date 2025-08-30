#pragma once

#include <thread>
#include <atomic>

#include <gst/gst.h>

class CameraService {
public:
    CameraService();
    ~CameraService();

    void start();
    void stop();

    void switch_to_camera();
    void switch_to_uri();

    GstElement* get_inference_appsink() { return nullptr; }

private:
    GstElement* build_pipeline();
    bool create_elements();
    void configure_elements();
    bool link_elements();
    void install_pad_probe();

    void bus_watch_function();

    static void on_pad_added(GstElement *src, GstPad *new_pad, gpointer user_data);
    void link_uridecodebin_pad(GstPad *new_pad);

    GstElement *pipeline_{nullptr}, *camera_src_{nullptr}, *camera_caps_nvmm_{nullptr}, *camera_conv_{nullptr}, *camera_caps_scaled_{nullptr};
//    GstElement *uri_src_{nullptr}, *uri_queue_{nullptr}, *uri_videorate_{nullptr}, *uri_caps_framerate_{nullptr}, *uri_conv_{nullptr}, *uri_caps_scaled_{nullptr}, *audio_fakesink_{nullptr};
//    GstElement *src_selector_{nullptr}, *tee_{nullptr};

    GstElement *front_queue_{nullptr}, *front_conv_{nullptr}, *front_caps_{nullptr}, *front_shm_{nullptr};
//    GstElement *inference_queue_{nullptr}, *inference_conv_{nullptr}, *inference_caps_nvmm_{nullptr}, *inference_streammux_{nullptr}, *inference_nvinfer_{nullptr}, \
//                *inference_conv3_{nullptr}, *inference_caps_sys_{nullptr}, *inference_appsink_{nullptr};

    GstBus* bus_{nullptr};
    std::thread bus_thread_;
    std::atomic<bool> is_active_{false};
};
