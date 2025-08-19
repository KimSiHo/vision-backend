#pragma once

#include <thread>
#include <atomic>

#include <gst/gst.h>

class CameraService {
public:
    CameraService();
    ~CameraService();

    void capture_on();
    void capture_off();

    GstElement* get_appsink() { return appsink_; }

private:
    GstElement* build_pipeline();
    bool create_elements();
    void configure_elements();
    bool link_elements();
    void install_pad_probe();

    void bus_watch_function();

    GstElement *pipeline_{nullptr}, *src_{nullptr}, *caps_nvmm_{nullptr}, *conv_{nullptr}, *caps_scaled_{nullptr}, *tee_{nullptr};
    GstElement *q1_{nullptr}, *conv_b1_{nullptr}, *caps_b1_{nullptr}, *shm_{nullptr};
    GstElement *q2_{nullptr}, *conv2_{nullptr}, *caps_nvmm_b2_{nullptr}, *streammux_{nullptr}, *appsink_{nullptr};
    GstElement *conv3_{nullptr}, *caps_sys_{nullptr};

    GstBus* bus_{nullptr};
    std::thread bus_thread_;
    std::atomic<bool> is_active_{false};
};
