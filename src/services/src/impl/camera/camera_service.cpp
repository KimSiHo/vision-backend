#include "services/camera/camera_service.hpp"

#include <spdlog/spdlog.h>
#include <nvdsmeta.h>
#include <nvbufsurface.h>
#include <gstnvdsmeta.h>

#include "common/utils/logging.hpp"

#define CHECK_ELEM(e, name) \
    if (!(e)) { \
        SPDLOG_SERVICE_ERROR("element creation failed: {}", name); \
        return NULL; \
    }

CameraService::CameraService() {
    pipeline_ = build_pipeline();
    if (!pipeline_) throw std::runtime_error("build_pipeline failed");
    bus_ = gst_element_get_bus(pipeline_);
    SPDLOG_SERVICE_INFO("[Camera] CameraService init success!");
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
	//switch_to_test();
    auto ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        SPDLOG_SERVICE_ERROR("[Camera] Failed to set PLAYING");
        gst_object_unref(bus_);
        bus_ = nullptr;
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
        return;
    }

    is_active_ = true;
    bus_thread_ = std::thread(&CameraService::bus_watch_function, this);
    SPDLOG_SERVICE_INFO("[Camera] Success to set PLAYING");
}

void CameraService::stop() {
    if (!is_active_) return;
    is_active_ = false;

    if (bus_thread_.joinable()) {
        if (bus_thread_.get_id() == std::this_thread::get_id()) {
            SPDLOG_SERVICE_WARN("[Camera] Detaching bus thread from itself.");
            bus_thread_.detach();
        } else {
            bus_thread_.join();
        }
    }

    gst_element_set_state(pipeline_, GST_STATE_NULL);
    SPDLOG_SERVICE_INFO("[Camera] Capture stopped");
}

void CameraService::switch_to_camera() {
    GstElement* selector = gst_bin_get_by_name(GST_BIN(pipeline_), "src_selector");

    if (selector) {
        GstPad* camera_pad = gst_element_get_static_pad(selector, "sink_0");
        g_object_set(selector, "active-pad", camera_pad, NULL);
        gst_object_unref(camera_pad);
        gst_object_unref(selector);
        SPDLOG_SERVICE_INFO("[Camera] Switched to camera source.");
    }

}

void CameraService::switch_to_test() {
    GstElement* selector = gst_bin_get_by_name(GST_BIN(pipeline_), "src_selector");

    if (selector) {
        GstPad* uri_pad = gst_element_get_static_pad(selector, "sink_1");
        g_object_set(selector, "active-pad", uri_pad, NULL);
        gst_object_unref(uri_pad);
        gst_object_unref(selector);
        SPDLOG_SERVICE_INFO("[Camera] Switched to URI source.");
		SPDLOG_SERVICE_INFO("Dumping pipeline graph to /tmp/pipeline_test.dot");
		GST_DEBUG_BIN_TO_DOT_FILE(
			GST_BIN(pipeline_),
			GST_DEBUG_GRAPH_SHOW_ALL,
			"pipeline_test" // 파일 이름 (pipeline_error.dot 으로 저장됨)
		);
    }
}

GstElement* CameraService::build_pipeline() {
    if (!create_elements()) {
        SPDLOG_SERVICE_ERROR("[Camera] Failed to create GStreamer elements.");
        return nullptr;
    }

    configure_elements();

    if (!link_elements()) {
        SPDLOG_SERVICE_ERROR("[Camera] Failed to link GStreamer elements.");
        return nullptr;
    }

    install_pad_probe();

    SPDLOG_SERVICE_INFO("[Camera] Pipeline built successfully.");
    return pipeline_;
}

bool CameraService::create_elements() {
    pipeline_ = gst_pipeline_new("inference-pipe");
    CHECK_ELEM(pipeline_, "pipeline")

    // 1) camera
    //camera_src_ = gst_element_factory_make("nvarguscamerasrc", "camera_src");
    //CHECK_ELEM(camera_src_, "nvarguscamerasrc")
    //camera_caps_nvmm_ = gst_element_factory_make("capsfilter", "camera_caps_nvmm");
    //CHECK_ELEM(camera_caps_nvmm_, "camera_caps_nvmm")
    //camera_conv_ = gst_element_factory_make("nvvideoconvert", "camera_conv");
    //CHECK_ELEM(camera_conv_, "camera_conv")
    //camera_caps_scaled_ = gst_element_factory_make("capsfilter", "camera_caps_scaled");
    //CHECK_ELEM(camera_caps_scaled_, "camera_caps_scaled")

    // 2) uri
    uri_src_ = gst_element_factory_make("uridecodebin", "uri_src");
    CHECK_ELEM(uri_src_, "uridecodebin")
    uri_queue_ = gst_element_factory_make("queue", "uri_queue");
    CHECK_ELEM(uri_queue_, "uri_queue")
    uri_conv_ = gst_element_factory_make("nvvideoconvert", "uri_conv");
    CHECK_ELEM(uri_conv_, "uri_conv")
    uri_caps_scaled_ = gst_element_factory_make("capsfilter", "uri_caps_scaled");
    CHECK_ELEM(uri_caps_scaled_, "uri_caps_scaled")
    uri_videorate_ = gst_element_factory_make("videorate", "front_videorate");
    CHECK_ELEM(uri_videorate_, "front_videorate")
    uri_caps_framerate_ = gst_element_factory_make("capsfilter", "front_caps_framerate");
    CHECK_ELEM(uri_caps_framerate_, "front_caps_framerate")
	uri_conv2_ = gst_element_factory_make("nvvideoconvert", "uri_conv2");
    CHECK_ELEM(uri_conv2_, "uri_conv2")
    uri_caps_scaled2_ = gst_element_factory_make("capsfilter", "uri_caps_scaled2");
    CHECK_ELEM(uri_caps_scaled2_, "uri_caps_scaled2")
    audio_fakesink_ = gst_element_factory_make("fakesink", "audio_discard");
    CHECK_ELEM(audio_fakesink_, "audio_discard")

    // 3) selector
    src_selector_ = gst_element_factory_make("input-selector", "src_selector");
    CHECK_ELEM(src_selector_, "input-selector")
    tee_ = gst_element_factory_make("tee", "tee");
    CHECK_ELEM(tee_, "tee")

    // 4) shm
    front_queue_ = gst_element_factory_make("queue", "front_queue");
    CHECK_ELEM(front_queue_, "front_queue")
    front_conv_ = gst_element_factory_make("nvvideoconvert", "front_conv");
    CHECK_ELEM(front_conv_, "front_conv")
    front_caps_ = gst_element_factory_make("capsfilter", "front_caps");
    CHECK_ELEM(front_caps_, "front_caps")
    front_shm_ = gst_element_factory_make("shmsink", "front_shm");
    CHECK_ELEM(front_shm_, "front_shm")

    // 5) inference
    inference_queue_ = gst_element_factory_make("queue", "q2");
    inference_conv_ = gst_element_factory_make("nvvideoconvert", "conv2");
    inference_caps_nvmm_ = gst_element_factory_make("capsfilter", "caps_nvmm_b2");
    inference_streammux_ = gst_element_factory_make("nvstreammux", "streammux");
    inference_nvinfer_ = gst_element_factory_make("nvinfer", "primary_gie");
    inference_conv3_ = gst_element_factory_make("nvvideoconvert", "conv3");
    inference_caps_sys_ = gst_element_factory_make("capsfilter", "caps_sys");
    inference_appsink_ = gst_element_factory_make("appsink", "inference_appsink");
//        !q2_ || !conv2_ || !caps_nvmm_b2_ || !streammux_ || !nvinfer_ || !conv3_ || !caps_sys_ || !appsink_
    //!camera_src_ || !camera_caps_nvmm_ || !camera_conv_ || !camera_caps_scaled_ ||
    if (!pipeline_ ||
        !uri_src_ || !uri_queue_ || !uri_conv_ || !uri_caps_scaled_ || !audio_fakesink_ ||
        !src_selector_ || !tee_ ||
        !front_queue_ || !front_conv_ || !front_caps_ || !uri_videorate_ || !uri_caps_framerate_ || !front_shm_) {
        SPDLOG_SERVICE_ERROR("[Camera] Failed to create one or more elements ");
        return false;
    }

    SPDLOG_SERVICE_INFO("[Camera] create_elements success");
    return true;
}

void CameraService::configure_elements() {
    // 1) camera
    // g_object_set(camera_src_, "sensor-id", 0, NULL);
//
    // GstCaps *caps_nvmm = gst_caps_from_string("video/x-raw(memory:NVMM),width=4608,height=2592,framerate=14/1");
    // g_object_set(camera_caps_nvmm_, "caps", caps_nvmm, NULL);
    // gst_caps_unref(caps_nvmm);
//
    // GstCaps *camera_caps_scaled = gst_caps_from_string("video/x-raw(memory:NVMM),width=1920,height=1080");
    // g_object_set(camera_caps_scaled_, "caps", camera_caps_scaled, NULL);
    // gst_caps_unref(camera_caps_scaled);

    // 2) uri
    GstCaps *caps = gst_caps_from_string("video/x-raw(memory:NVMM)");
    //http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/WhatCarCanYouGetForAGrand.mp4

    g_object_set(uri_src_, "uri", "https://cdn.pixabay.com/video/2016/02/14/2165-155327596_large.mp4", "caps", caps, NULL);
    gst_caps_unref(caps);

    GstCaps *uri_caps_scaled = gst_caps_from_string("video/x-raw,format=NV12,width=1920,height=1080");
    g_object_set(uri_caps_scaled_, "caps", uri_caps_scaled, NULL);
    gst_caps_unref(uri_caps_scaled);


    // 4) shm
    g_object_set(front_queue_,
                "max-size-buffers", 1,
                "leaky", 2, NULL);

    GstCaps *front_caps_scaled = gst_caps_from_string("video/x-raw,format=I420,width=960,height=544");
    g_object_set(front_caps_, "caps", front_caps_scaled, NULL);
    gst_caps_unref(front_caps_scaled);

    g_object_set(front_shm_,
                "socket-path", "/tmp/cam.sock",
                "sync", FALSE,
                "shm-size", 3145728,
                "wait-for-connection", FALSE, NULL);

    SPDLOG_SERVICE_INFO("[Camera] configure_elements success");

    // 5) inference
    g_object_set(inference_queue_,
                "max-size-buffers", 5,
                "leaky", 1, NULL);

    GstCaps *caps_b2 = gst_caps_from_string("video/x-raw(memory:NVMM),format=NV12,width=960,height=544");
    g_object_set(inference_caps_nvmm_, "caps", caps_b2, NULL);
    gst_caps_unref(caps_b2);

    g_object_set(inference_streammux_,
                "batch-size", 1,
                "width", 960,
                "height", 544,
                "live-source", TRUE,
                "batched-push-timeout", 33000, NULL);

    g_object_set(inference_nvinfer_,
                "config-file-path", "/etc/vision-backend/config_infer_primary.txt", NULL);

    GstCaps *caps_sys = gst_caps_from_string("video/x-raw,format=RGBA");
    g_object_set(inference_caps_sys_, "caps", caps_sys, NULL);
    gst_caps_unref(caps_sys);

    g_object_set(inference_appsink_,
                "emit-signals", true,
                "max-buffers", 1,
                "drop", TRUE, NULL);
}

bool CameraService::link_elements() {
//    camera_src_, camera_caps_nvmm_, camera_conv_, camera_caps_scaled_,

    gst_bin_add_many(GST_BIN(pipeline_),
                    src_selector_, tee_,
                    uri_src_, uri_queue_, uri_conv_, uri_caps_scaled_, audio_fakesink_,
                    front_queue_, front_conv_, front_caps_, front_shm_,
					inference_queue_, inference_conv_, inference_streammux_, inference_nvinfer_,
	                inference_conv3_, inference_caps_sys_, inference_appsink_, NULL);

    // 1) camera
  //  if (!gst_element_link_many(camera_src_, camera_caps_nvmm_, camera_conv_, camera_caps_scaled_, NULL)) {
  //      SPDLOG_SERVICE_ERROR("[Camera] Failed to link camera source");
  //      return false;
  //  }

    //GstPad* camera_caps_scaled_src = gst_element_get_static_pad(camera_caps_scaled_, "src");
    //GstPad* selector_sink0 = gst_element_request_pad_simple(src_selector_, "sink_%u");
    //if (gst_pad_link(camera_caps_scaled_src, selector_sink0) != GST_PAD_LINK_OK) {
    //    SPDLOG_SERVICE_ERROR("[Camera] Failed to link camera source to input selector sink_0");
    //    return false;
    //}
    //gst_object_unref(camera_caps_scaled_src);
    //gst_object_unref(selector_sink0);

    // 2) uri
    g_signal_connect(G_OBJECT(uri_src_), "pad-added", G_CALLBACK(on_pad_added), this);
    if (!gst_element_link_many(uri_queue_, uri_conv_, uri_caps_scaled_, NULL)) {
        SPDLOG_SERVICE_ERROR("[Camera] Failed to link uri source");
        return false;
    }

    g_signal_connect(uri_src_, "autoplug-continue",
                     G_CALLBACK(on_autoplug_continue), NULL);

    GstPad* uri_caps_scaled_src = gst_element_get_static_pad(uri_caps_scaled_, "src");
    GstPad* selector_sink1 = gst_element_request_pad_simple(src_selector_, "sink_%u");
    if (gst_pad_link(uri_caps_scaled_src, selector_sink1) != GST_PAD_LINK_OK) {
        SPDLOG_SERVICE_ERROR("[Camera] Failed to link uri source to input selector sink_1");
        return false;
    }
    gst_object_unref(uri_caps_scaled_src);
    gst_object_unref(selector_sink1);

    // 3) selector / tee
    if (!gst_element_link(src_selector_, tee_)) {
        SPDLOG_SERVICE_ERROR("[Camera] Failed to link input_selector to tee");
        return false;
    }

    GstPad *tee_src0 = gst_element_request_pad_simple(tee_, "src_%u");
    GstPad *tee_src1 = gst_element_request_pad_simple(tee_, "src_%u");
    GstPad *front_queue_sink  = gst_element_get_static_pad(front_queue_, "sink");
    GstPad *inference_queue_sink  = gst_element_get_static_pad(inference_queue_, "sink");

    if (gst_pad_link(tee_src0, front_queue_sink) != GST_PAD_LINK_OK ||
        gst_pad_link(tee_src1, inference_queue_sink) != GST_PAD_LINK_OK) {
        SPDLOG_SERVICE_ERROR("[Camera] link elements tee -> queue fail!");
		return false;
	}


    gst_object_unref(tee_src0);
    gst_object_unref(tee_src1);
    gst_object_unref(front_queue_sink);
    gst_object_unref(inference_queue_sink);

    // 4) shm
    if (!gst_element_link_many(front_queue_, front_conv_, front_caps_, front_shm_, NULL)) {
        SPDLOG_SERVICE_ERROR("[Camera] link elements front_queue -> front_shm fail!");
        return false;
    }

    // 5) inference
    if (!gst_element_link_many(inference_queue_, inference_conv_, NULL)) {
        SPDLOG_SERVICE_ERROR("[Camera] link elements q2 -> caps_nvmm fail!");
        return false;
    }

    GstPad *mux_sink0 = gst_element_request_pad_simple(inference_streammux_, "sink_0");
    GstPad *caps_src  = gst_element_get_static_pad(inference_conv_, "src");
    if (gst_pad_link(caps_src, mux_sink0) != GST_PAD_LINK_OK) {
        SPDLOG_SERVICE_ERROR("[Camera] link elements caps_src -> mux_sink fail!");
        return false;
    }

    gst_object_unref(caps_src);
    gst_object_unref(mux_sink0);

    if (!gst_element_link_many(inference_streammux_, inference_nvinfer_, inference_conv3_, inference_caps_sys_, inference_appsink_, NULL)) {
        return false;
    }

    return true;
}

static gboolean is_audio_caps (GstCaps *caps) {
    const GstStructure *s = gst_caps_get_structure(caps, 0);
    const gchar *name = gst_structure_get_name(s);
    return g_str_has_prefix(name, "audio/");
}


gboolean CameraService::on_autoplug_continue(GstElement *bin,
                                     GstPad     *pad,
                                     GstCaps    *caps,
                                     gpointer    user_data) {
    gchar *caps_str = gst_caps_to_string(caps);
    g_print("[uridecodebin] new stream on pad '%s': %s\n",
            GST_PAD_NAME(pad), caps_str);

    if (is_audio_caps(caps)) {
        g_print("[decodebin] skip audio stream\n");
        return FALSE; // 디코더 찾기 시도 자체를 중단(압축 오디오 pad를 그대로 노출)
    }

    g_free(caps_str);

    // TRUE  : uridecodebin이 이 스트림에 맞는 디코더/후속 요소를 계속 자동으로 붙임
    // FALSE : 여기서 자동 연결 중단하고, 지금 caps로 pad를 그대로 외부로 노출
    return TRUE;   // 우선 계속 자동 플러깅 하려면 TRUE
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

                SPDLOG_SERVICE_ERROR("[Camera] GStreamer ERROR from element [{}]: {}",
                                (elem_name ? elem_name : "unknown"),
                                (err ? err->message : "unknown"));
                SPDLOG_SERVICE_ERROR("[Camera] Debugging information: {}", (dbg ? dbg : "none"));

                if (dbg) g_free(dbg);
                if (err) g_error_free(err);

                SPDLOG_SERVICE_INFO("Dumping pipeline graph to /tmp/pipeline_error.dot");
                GST_DEBUG_BIN_TO_DOT_FILE(
                    GST_BIN(pipeline_),
                    GST_DEBUG_GRAPH_SHOW_ALL,
                    "pipeline_error" // 파일 이름 (pipeline_error.dot 으로 저장됨)
                );

                stop();
                break;
            }

            case GST_MESSAGE_EOS: {
                SPDLOG_SERVICE_ERROR("[Camera] EOS received");

                stop();
                break;
            }

            default: {
                break;
            }
        }

        gst_message_unref(msg);
    }

    SPDLOG_SERVICE_INFO("[Camera] Bus watch thread finished.");
}

void CameraService::on_pad_added(GstElement *src, GstPad *new_pad, gpointer user_data) {
    CameraService *self = static_cast<CameraService*>(user_data);
    SPDLOG_SERVICE_INFO("[Camera] uridecodebin created a new pad '{}'", GST_PAD_NAME(new_pad));

    self->link_uridecodebin_pad(new_pad);
}

void CameraService::link_uridecodebin_pad(GstPad *new_pad) {
    GstCaps *new_pad_caps = gst_pad_get_current_caps(new_pad);
    GstStructure *new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    const gchar *new_pad_type = gst_structure_get_name(new_pad_struct);
    SPDLOG_SERVICE_INFO("uridecodebin created a new pad of type '{}'", new_pad_type);

    GstPad *sink_pad = nullptr; // 연결할 sink_pad를 담을 변수

    // 비디오 패드인 경우, sink_pad를 uri_queue_로 설정
    if (g_str_has_prefix(new_pad_type, "video/x-raw")) {
        SPDLOG_SERVICE_INFO("[Camera] Found video pad, preparing to link to queue.");
        sink_pad = gst_element_get_static_pad(uri_queue_, "sink");
    } else if (g_str_has_prefix(new_pad_type, "audio/")) {
        SPDLOG_SERVICE_INFO("[Camera] Found audio pad, preparing to link to fakesink.");
        sink_pad = gst_element_get_static_pad(audio_fakesink_, "sink");
    } else {
        SPDLOG_SERVICE_WARN("[Camera] Ignoring unknown pad type from uridecodebin: {}", new_pad_type);
    }

    // sink_pad가 결정되었다면 (비디오 또는 오디오였다면) 연결 로직 실행
    if (sink_pad) {
        if (gst_pad_is_linked(sink_pad)) {
            SPDLOG_SERVICE_WARN("Sink pad is already linked.");
        } else {
            if (gst_pad_link(new_pad, sink_pad) != GST_PAD_LINK_OK) {
                SPDLOG_SERVICE_ERROR("Failed to link new pad.");
            } else {
                SPDLOG_SERVICE_INFO("Successfully linked new pad.");
            }
        }
        // 사용이 끝난 sink_pad는 항상 unref
        gst_object_unref(sink_pad);
    }

    if (new_pad_caps != nullptr) {
        gst_caps_unref(new_pad_caps);
    }
}
