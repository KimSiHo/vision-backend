#include "camera_service.hpp"

#include <spdlog/spdlog.h>
#include <nvdsmeta.h>
#include <nvbufsurface.h>
#include <gstnvdsmeta.h>

#define CHECK_ELEM(e, name) \
    if (!(e)) { \
        gst_printerr("element creation failed: %s\n", name); \
        return NULL; \
    }

CameraService::CameraService() {
    pipeline_ = build_pipeline();
    if (!pipeline_) throw std::runtime_error("build_pipeline failed");
    bus_ = gst_element_get_bus(pipeline_);
    spdlog::info("[Camera] CameraService init success!");
}

CameraService::~CameraService() {
    capture_off();
    if (bus_) {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }
    if (pipeline_) {
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
}

void CameraService::capture_on() {
    auto ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        spdlog::error("[Camera] Failed to set PLAYING");
        gst_object_unref(bus_);
        bus_ = nullptr;
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
        return;
    }

    is_active_ = true;
    bus_thread_ = std::thread(&CameraService::bus_watch_function, this);
    spdlog::info("[Camera] Success to set PLAYING");
}

void CameraService::capture_off() {
   if (!is_active_) return;
    is_active_ = false;

    if (bus_thread_.joinable()) {
        if (bus_thread_.get_id() == std::this_thread::get_id()) {
            spdlog::warn("[Camera] Detaching bus thread from itself.");
            bus_thread_.detach();
        } else {
            bus_thread_.join();
        }
    }

    gst_element_set_state(pipeline_, GST_STATE_NULL);
    spdlog::info("[Camera] Capture stopped");
}

void CameraService::bus_watch_function() {
    while (is_active_) {
        GstMessage* msg = gst_bus_timed_pop_filtered(
            bus_,
            100 * GST_MSECOND,
            static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        if (!msg) continue;

        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR: {
                GError* err = nullptr;
                gchar* dbg = nullptr;

                gst_message_parse_error(msg, &err, &dbg);
                const gchar* elem_name = GST_OBJECT_NAME(GST_MESSAGE_SRC(msg));

                spdlog::error("[Camera] GStreamer ERROR from element [{}]: {}",
                                (elem_name ? elem_name : "unknown"),
                                (err ? err->message : "unknown"));
                spdlog::error("[Camera] Debugging information: {}", (dbg ? dbg : "none"));

                if (dbg) g_free(dbg);
                if (err) g_error_free(err);

                capture_off();
                break;
            }

            case GST_MESSAGE_EOS: {
                spdlog::error("[Camera] EOS received");
                capture_off();
                break;
            }

            default: {
                break;
            }
        }

        gst_message_unref(msg);
    }

    spdlog::info("[Camera] Bus watch thread finished.");
}

GstElement* CameraService::build_pipeline() {
    if (!create_elements()) {
        spdlog::error("[Camera] Failed to create GStreamer elements.");
        return nullptr;
    }

    configure_elements();

    if (!link_elements()) {
        spdlog::error("[Camera] Failed to link GStreamer elements.");
        return nullptr;
    }

    install_pad_probe();

    spdlog::info("[Camera] Pipeline built successfully.");
    return pipeline_;
}

bool CameraService::create_elements() {
    pipeline_ = gst_pipeline_new("cam-pipe");
    src_ = gst_element_factory_make("nvarguscamerasrc", "src");
    caps_nvmm_ = gst_element_factory_make("capsfilter", "caps_nvmm");
    conv_ = gst_element_factory_make("nvvideoconvert", "conv");
    caps_scaled_ = gst_element_factory_make("capsfilter", "caps_scaled");
    tee_ = gst_element_factory_make("tee", "t");

    q1_ = gst_element_factory_make("queue", "q1");
    conv_b1_ = gst_element_factory_make("nvvideoconvert", "conv_b1");
    caps_b1_ = gst_element_factory_make("capsfilter", "caps_b1");
    shm_ = gst_element_factory_make("shmsink", "shm");

    q2_ = gst_element_factory_make("queue", "q2");
    conv2_ = gst_element_factory_make("nvvideoconvert", "conv2");
    caps_nvmm_b2_ = gst_element_factory_make("capsfilter", "caps_nvmm_b2");
    streammux_ = gst_element_factory_make("nvstreammux", "streammux");

    conv3_ = gst_element_factory_make("nvvideoconvert", "conv3");
    caps_sys_ = gst_element_factory_make("capsfilter", "caps_sys");
    appsink_ = gst_element_factory_make("appsink", "appsink");

    if (!pipeline_ || !src_ || !caps_nvmm_ || !conv_ || !caps_scaled_ || !tee_ ||
        !q1_ || !conv_b1_ || !caps_b1_ || !shm_ ||
        !q2_ || !conv2_ || !caps_nvmm_b2_ || !streammux_ ||
        !conv3_ || !caps_sys_ || !appsink_) {
        return false;
    }
    return true;
}

void CameraService::configure_elements() {
    g_object_set(src_, "sensor-id", 0, NULL);

    GstCaps *caps0 = gst_caps_from_string("video/x-raw(memory:NVMM),width=4608,height=2592,framerate=14/1");
    g_object_set(caps_nvmm_, "caps", caps0, NULL);
    gst_caps_unref(caps0);

    GstCaps *caps_scaled = gst_caps_from_string("video/x-raw(memory:NVMM),width=1920,height=1080");
    g_object_set(caps_scaled_, "caps", caps_scaled, NULL);
    gst_caps_unref(caps_scaled);

    GstCaps *caps1 = gst_caps_from_string("video/x-raw,format=I420,width=640,height=480,framerate=14/1");
    g_object_set(caps_b1_, "caps", caps1, NULL);
    gst_caps_unref(caps1);

    g_object_set(shm_,
                "socket-path", "/tmp/cam.sock",
                "sync", FALSE,
                "wait-for-connection", FALSE, NULL);

    GstCaps *caps_b2 = gst_caps_from_string("video/x-raw(memory:NVMM),format=NV12,width=1280,height=720,framerate=14/1");
    g_object_set(caps_nvmm_b2_, "caps", caps_b2, NULL);
    gst_caps_unref(caps_b2);

    g_object_set(streammux_,
                "batch-size", 1,
                "width", 1280,
                "height", 720,
                "live-source", TRUE,
                "batched-push-timeout", 33000, NULL);

    GstCaps *caps_sys = gst_caps_from_string("video/x-raw,format=RGBA");
    g_object_set(caps_sys_, "caps", caps_sys, NULL);
    gst_caps_unref(caps_sys);

    g_object_set(appsink_,
                "emit-signals", FALSE,
                "max-buffers", 1,
                "drop", TRUE, NULL);
}

bool CameraService::link_elements() {
    gst_bin_add_many(GST_BIN(pipeline_),
                     src_, caps_nvmm_, conv_, caps_scaled_, tee_,
                     q1_, conv_b1_, caps_b1_, shm_,
                     q2_, conv2_, caps_nvmm_b2_, streammux_,
                     conv3_, caps_sys_, appsink_, NULL);
    bool result = true;

    if (!gst_element_link_many(src_, caps_nvmm_, conv_, caps_scaled_, tee_, NULL)) {
        spdlog::error("[Camera] link src -> tee fail!");
        result = false;
    }

    GstPad *tee_src0 = gst_element_request_pad_simple(tee_, "src_%u");
    GstPad *tee_src1 = gst_element_request_pad_simple(tee_, "src_%u");
    GstPad *q1_sink  = gst_element_get_static_pad(q1_, "sink");
    GstPad *q2_sink  = gst_element_get_static_pad(q2_, "sink");

    if (gst_pad_link(tee_src0, q1_sink) != GST_PAD_LINK_OK ||
        gst_pad_link(tee_src1, q2_sink) != GST_PAD_LINK_OK) {
            spdlog::error("[Camera] link elements tee -> queue fail!");
            result = false;
    }

    gst_object_unref(tee_src0);
    gst_object_unref(tee_src1);
    gst_object_unref(q1_sink);
    gst_object_unref(q2_sink);

    if (!gst_element_link_many(q1_, conv_b1_, caps_b1_, shm_, NULL)) {
        spdlog::error("[Camera] link elements q1 -> shm fail!");
        result = false;
    }

    if (!gst_element_link_many(q2_, conv2_, caps_nvmm_b2_, NULL)) {
        spdlog::error("[Camera] link elements q2 -> caps_nvmm fail!");
        result = false;
    }

    GstPad *mux_sink0 = gst_element_request_pad_simple(streammux_, "sink_0");
    GstPad *caps_src  = gst_element_get_static_pad(caps_nvmm_b2_, "src");
    if (gst_pad_link(caps_src, mux_sink0) != GST_PAD_LINK_OK) {
        spdlog::error("[Camera] link elements caps_src -> mux_sink fail!");
        result = false;
    }

    gst_object_unref(caps_src);
    gst_object_unref(mux_sink0);

    if (!gst_element_link_many(streammux_, conv3_, caps_sys_, appsink_, NULL)) {
        result = false;
    }

    return result;
}

void CameraService::install_pad_probe() {
    GstPad *mux_src = gst_element_get_static_pad(streammux_, "src");
    gst_pad_add_probe(mux_src,
        GST_PAD_PROBE_TYPE_BUFFER,
        [](GstPad*, GstPadProbeInfo* info, gpointer) -> GstPadProbeReturn {
            // 필요 시 버퍼 검사/타임스탬프 로깅 등
            return GST_PAD_PROBE_OK;
        },
        nullptr,
        nullptr);
    gst_object_unref(mux_src);
}
