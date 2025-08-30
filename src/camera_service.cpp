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
    stop();
    if (bus_) {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }
    if (pipeline_) {
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
}

void CameraService::start() {
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

void CameraService::stop() {
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

void CameraService::switch_to_camera() {
    GstElement* selector = gst_bin_get_by_name(GST_BIN(pipeline_), "src_selector");

    if (selector) {
        GstPad* camera_pad = gst_element_get_static_pad(selector, "sink_0");
        g_object_set(selector, "active-pad", camera_pad, NULL);
        gst_object_unref(camera_pad);
        gst_object_unref(selector);
        spdlog::info("[Camera] Switched to camera source.");
    }
}

void CameraService::switch_to_uri() {
    GstElement* selector = gst_bin_get_by_name(GST_BIN(pipeline_), "src_selector");

    if (selector) {
        GstPad* uri_pad = gst_element_get_static_pad(selector, "sink_1");
        g_object_set(selector, "active-pad", uri_pad, NULL);
        gst_object_unref(uri_pad);
        gst_object_unref(selector);
        spdlog::info("[Camera] Switched to URI source.");
    }
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
    pipeline_ = gst_pipeline_new("inference-pipe");

    // 1) camera
    camera_src_ = gst_element_factory_make("nvarguscamerasrc", "camera_src");
    camera_caps_nvmm_ = gst_element_factory_make("capsfilter", "camera_caps_nvmm");
    camera_conv_ = gst_element_factory_make("nvvideoconvert", "camera_conv");
    camera_caps_scaled_ = gst_element_factory_make("capsfilter", "camera_caps_scaled");

    // 2) uri
//    uri_src_ = gst_element_factory_make("uridecodebin", "uri_src");
//    uri_queue_ = gst_element_factory_make("queue", "uri_queue");
//     uri_videorate_ = gst_element_factory_make("videorate", "uri_videorate");
//    uri_caps_framerate_ = gst_element_factory_make("capsfilter", "uri_caps_framerate");
//    uri_conv_ = gst_element_factory_make("nvvideoconvert", "uri_conv");
//    uri_caps_scaled_ = gst_element_factory_make("capsfilter", "uri_caps_scaled");
//    audio_fakesink_ = gst_element_factory_make("fakesink", "audio_discard");
//
//    // 3) selector
//    src_selector_ = gst_element_factory_make("input-selector", "src_selector");
//    tee_ = gst_element_factory_make("tee", "tee");

    // 4) shm
    front_queue_ = gst_element_factory_make("queue", "front_queue");
    front_conv_ = gst_element_factory_make("nvvideoconvert", "front_conv");
    front_caps_ = gst_element_factory_make("capsfilter", "front_caps");
    front_shm_ = gst_element_factory_make("shmsink", "front_shm");

    // 5) inference
//    q2_ = gst_element_factory_make("queue", "q2");
//    conv2_ = gst_element_factory_make("nvvideoconvert", "conv2");
//    caps_nvmm_b2_ = gst_element_factory_make("capsfilter", "caps_nvmm_b2");
//    streammux_ = gst_element_factory_make("nvstreammux", "streammux");
//    nvinfer_ = gst_element_factory_make("nvinfer", "primary_gie");
//    conv3_ = gst_element_factory_make("nvvideoconvert", "conv3");
//    caps_sys_ = gst_element_factory_make("capsfilter", "caps_sys");
//    inference_appsink_ = gst_element_factory_make("appsink", "inference_appsink");
//        !q2_ || !conv2_ || !caps_nvmm_b2_ || !streammux_ || !nvinfer_ || !conv3_ || !caps_sys_ || !appsink_
//    if (!pipeline_ || !camera_src_ || !camera_caps_nvmm_ || !camera_conv_ || !camera_caps_scaled_ ||
//        !uri_src_ || !uri_queue_ || !uri_videorate_ || !uri_caps_framerate_ || !uri_conv_ || !uri_caps_scaled_ ||
//        !src_selector_ || !tee_ ||
//        !front_queue_ || !front_conv_ || !front_caps_ || !front_shm_) {
//        spdlog::error("[Camera] Failed to create one or more elements");
//        return false;
//    }

    spdlog::info("[Camera] create_elements success");
    return true;
}

void CameraService::configure_elements() {
    // 1) camera
    g_object_set(camera_src_, "sensor-id", 0, NULL);

    GstCaps *caps_nvmm = gst_caps_from_string("video/x-raw(memory:NVMM),width=4608,height=2592,framerate=14/1");
    g_object_set(camera_caps_nvmm_, "caps", caps_nvmm, NULL);
    gst_caps_unref(caps_nvmm);

    GstCaps *camera_caps_scaled = gst_caps_from_string("video/x-raw(memory:NVMM),width=1920,height=1080,framerate=14/1");
    g_object_set(camera_caps_scaled_, "caps", camera_caps_scaled, NULL);
    gst_caps_unref(camera_caps_scaled);

//    // 2) uri
//    g_object_set(uri_src_, "uri", "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/WhatCarCanYouGetForAGrand.mp4", NULL);
//
//    GstCaps *uri_framerate_caps = gst_caps_from_string("video/x-raw,framerate=14/1");
//    g_object_set(uri_caps_framerate_, "caps", uri_framerate_caps, NULL);
//    gst_caps_unref(uri_framerate_caps);
//
//    GstCaps *uri_caps_scaled = gst_caps_from_string("video/x-raw(memory:NVMM),width=1920,height=1080,framerate=14/1");
//    g_object_set(uri_caps_scaled_, "caps", uri_caps_scaled, NULL);
//    gst_caps_unref(uri_caps_scaled);

    // 4) shm
    g_object_set(front_queue_,
                "max-size-buffers", 2,
                "leaky", 2, NULL);

    GstCaps *front_caps_scaled = gst_caps_from_string("video/x-raw,format=I420,width=960,height=540,framerate=14/1");
    g_object_set(front_caps_, "caps", front_caps_scaled, NULL);
    gst_caps_unref(front_caps_scaled);

    g_object_set(front_shm_,
                "socket-path", "/tmp/cam.sock",
                "sync", TRUE,
                "shm-size", 50331648,
                "wait-for-connection", FALSE, NULL);

    spdlog::info("[Camera] configure_elements success");

    // 5) inference
//    g_object_set(q2_,
//                "max-size-buffers", 5,
//                "leaky", 1, NULL);
//
//    GstCaps *caps_b2 = gst_caps_from_string("video/x-raw(memory:NVMM),format=NV12,width=1280,height=720,framerate=14/1");
//    g_object_set(caps_nvmm_b2_, "caps", caps_b2, NULL);
//    gst_caps_unref(caps_b2);
//
//    g_object_set(streammux_,
//                "batch-size", 1,
//                "width", 1280,
//                "height", 720,
//                "live-source", TRUE,
//                "batched-push-timeout", 33000, NULL);
//
//    g_object_set(nvinfer_,
//                "config-file-path", "/etc/vision-backend/config_infer_primary.txt", NULL);
//
//    GstCaps *caps_sys = gst_caps_from_string("video/x-raw,format=RGBA");
//    g_object_set(caps_sys_, "caps", caps_sys, NULL);
//    gst_caps_unref(caps_sys);
//
//    g_object_set(appsink_,
//                "emit-signals", true,
//                "max-buffers", 1,
//                "drop", TRUE, NULL);
}

bool CameraService::link_elements() {
//                    q2_, conv2_, caps_nvmm_b2_, streammux_, nvinfer_, conv3_, caps_sys_, appsink_, NULL

    gst_bin_add_many(GST_BIN(pipeline_),
                    camera_src_, camera_caps_nvmm_, camera_conv_, camera_caps_scaled_,
                    front_queue_, front_conv_, front_caps_, front_shm_, NULL);

    // 1) camera
    if (!gst_element_link_many(camera_src_, camera_caps_nvmm_, camera_conv_, camera_caps_scaled_, NULL)) {
        spdlog::error("[Camera] Failed to link camera source");
        return false;
    }

//    GstPad* camera_caps_scaled_src = gst_element_get_static_pad(camera_caps_scaled_, "src");
//    GstPad* selector_sink0 = gst_element_request_pad_simple(src_selector_, "sink_%u");
//    if (gst_pad_link(camera_caps_scaled_src, selector_sink0) != GST_PAD_LINK_OK) {
//        spdlog::error("[Camera] Failed to link camera source to input selector sink_0");
//        return false;
//    }
//    gst_object_unref(camera_caps_scaled_src);
//    gst_object_unref(selector_sink0);

    // 2) uri
//    g_signal_connect(G_OBJECT(uri_src_), "pad-added", G_CALLBACK(on_pad_added), this);
//    if (!gst_element_link_many(uri_queue_, uri_videorate_, uri_caps_framerate_, uri_conv_, uri_caps_scaled_, NULL)) {
//        spdlog::error("[Camera] Failed to link uri source");
//        return false;
//    }
//
//    GstPad* uri_caps_scaled_src = gst_element_get_static_pad(uri_caps_scaled_, "src");
//    GstPad* selector_sink1 = gst_element_request_pad_simple(src_selector_, "sink_%u");
//    if (gst_pad_link(uri_caps_scaled_src, selector_sink1) != GST_PAD_LINK_OK) {
//        spdlog::error("[Camera] Failed to link uri source to input selector sink_1");
//        return false;
//    }
//    gst_object_unref(uri_caps_scaled_src);
//    gst_object_unref(selector_sink1);
//
//    // 3) selector / tee
//    if (!gst_element_link(src_selector_, tee_)) {
//        spdlog::error("[Camera] Failed to link input_selector to tee");
//        return false;
//    }
//
//    GstPad *tee_src0 = gst_element_request_pad_simple(tee_, "src_%u");
//    GstPad *tee_src1 = gst_element_request_pad_simple(tee_, "src_%u");
//    GstPad *front_queue_sink  = gst_element_get_static_pad(front_queue_, "sink");
    //GstPad *inference_queue_sink  = gst_element_get_static_pad(inference_queue_, "sink");

//if (gst_pad_link(tee_src0, front_queue_sink) != GST_PAD_LINK_OK ||
//        gst_pad_link(tee_src1, inference_queue_sink) != GST_PAD_LINK_OK) {

//    if (gst_pad_link(tee_src0, front_queue_sink) != GST_PAD_LINK_OK) {
//        spdlog::error("[Camera] link elements tee -> queue fail!");
//        return false;
//    }
//    gst_object_unref(tee_src0);
//    gst_object_unref(tee_src1);
//    gst_object_unref(front_queue_sink);
//    //gst_object_unref(inference_queue_sink);

    // 4) shm
    if (!gst_element_link_many(front_queue_, front_conv_, front_caps_, front_shm_, NULL)) {
        spdlog::error("[Camera] link elements front_queue -> front_shm fail!");
        return false;
    }

    // 5) inference
//    if (!gst_element_link_many(q2_, conv2_, caps_nvmm_b2_, NULL)) {
//        spdlog::error("[Camera] link elements q2 -> caps_nvmm fail!");
//        return false;
//    }
//
//    GstPad *mux_sink0 = gst_element_request_pad_simple(streammux_, "sink_0");
//    GstPad *caps_src  = gst_element_get_static_pad(caps_nvmm_b2_, "src");
//    if (gst_pad_link(caps_src, mux_sink0) != GST_PAD_LINK_OK) {
//        spdlog::error("[Camera] link elements caps_src -> mux_sink fail!");
//        return false;
//    }
//
//    gst_object_unref(caps_src);
//    gst_object_unref(mux_sink0);
//
//    if (!gst_element_link_many(streammux_, nvinfer_, conv3_, caps_sys_, appsink_, NULL)) {
//        return false;
//    }

    return true;
}

void CameraService::install_pad_probe() {
//    GstPad *mux_src = gst_element_get_static_pad(streammux_, "src");
//    gst_pad_add_probe(mux_src,
//        GST_PAD_PROBE_TYPE_BUFFER,
//        [](GstPad*, GstPadProbeInfo* info, gpointer) -> GstPadProbeReturn {
//            // 필요 시 버퍼 검사/타임스탬프 로깅 등
//            return GST_PAD_PROBE_OK;
//        },
//        nullptr,
//        nullptr);
//    gst_object_unref(mux_src);
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

                spdlog::info("Dumping pipeline graph to /tmp/pipeline_error.dot");
                GST_DEBUG_BIN_TO_DOT_FILE(
                    GST_BIN(pipeline_),
                    GST_DEBUG_GRAPH_SHOW_ALL,
                    "pipeline_error" // 파일 이름 (pipeline_error.dot 으로 저장됨)
                );

                stop();
                break;
            }

            case GST_MESSAGE_EOS: {
                spdlog::error("[Camera] EOS received");

                stop();
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

//void CameraService::on_pad_added(GstElement *src, GstPad *new_pad, gpointer user_data) {
//    CameraService *self = static_cast<CameraService*>(user_data);
//    spdlog::info("[Camera] uridecodebin created a new pad '{}'", GST_PAD_NAME(new_pad));
//
//    self->link_uridecodebin_pad(new_pad);
//}
//
//void CameraService::link_uridecodebin_pad(GstPad *new_pad) {
//    GstCaps *new_pad_caps = gst_pad_get_current_caps(new_pad);
//    GstStructure *new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
//    const gchar *new_pad_type = gst_structure_get_name(new_pad_struct);
//
//    GstPad *sink_pad = nullptr; // 연결할 sink_pad를 담을 변수
//
//    // 비디오 패드인 경우, sink_pad를 uri_queue_로 설정
//    if (g_str_has_prefix(new_pad_type, "video/x-raw")) {
//        spdlog::info("[Camera] Found video pad, preparing to link to queue.");
//        sink_pad = gst_element_get_static_pad(uri_queue_, "sink");
//    } else if (g_str_has_prefix(new_pad_type, "audio/x-raw")) {
//        spdlog::info("[Camera] Found audio pad, preparing to link to fakesink.");
//        sink_pad = gst_element_get_static_pad(audio_fakesink_, "sink");
//    } else {
//        spdlog::warn("[Camera] Ignoring unknown pad type from uridecodebin: {}", new_pad_type);
//    }
//
//    // sink_pad가 결정되었다면 (비디오 또는 오디오였다면) 연결 로직 실행
//    if (sink_pad) {
//        if (gst_pad_is_linked(sink_pad)) {
//            spdlog::warn("Sink pad is already linked.");
//        } else {
//            if (gst_pad_link(new_pad, sink_pad) != GST_PAD_LINK_OK) {
//                spdlog::error("Failed to link new pad.");
//            } else {
//                spdlog::info("Successfully linked new pad.");
//            }
//        }
//        // 사용이 끝난 sink_pad는 항상 unref
//        gst_object_unref(sink_pad);
//    }
//
//    if (new_pad_caps != nullptr) {
//        gst_caps_unref(new_pad_caps);
//    }
//}
