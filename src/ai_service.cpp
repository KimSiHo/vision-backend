#include "ai_service.hpp"

#include <chrono>

#include <nvdsmeta.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <vision_common/constants.hpp>
#include <gst/video/video.h>

using json = nlohmann::json;

AiService::AiService(zmq::context_t& ctx, tbb::concurrent_queue<int>& q, GstElement* appsink_elem)
    : pub_(ctx, std::string(VisionCommon::AI_RESULTS_ENDPOINT)), job_queue_(q) {
        attach(appsink_elem);
    }

AiService::~AiService() {
    stop();
    detach();
}

void AiService::start() {
    running_ = true;
    processing_thread_ = std::thread([this] { run(); });
}

void AiService::stop() {
    running_ = false;
    if (processing_thread_.joinable()) processing_thread_.join();
}

void AiService::run() {
    spdlog::info("[AI] Service ready (publishing results)");
    int job;
    json inference_result;

    while (running_) {
        inference_result = {{"ok", true}};
        pub_.publish(std::string(VisionCommon::TOPIC_DETECTIONS), inference_result.dump());
        if (job_queue_.try_pop(job)) {
            //spdlog::info("[AI] got job {}", job);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void AiService::attach(GstElement* appsink_elem) {
    detach();
    if (!appsink_elem) return;

    sink_ = GST_APP_SINK(gst_object_ref(appsink_elem));

    GstAppSinkCallbacks cbs{};
    cbs.new_sample = &AiService::on_new_sample;
    gst_app_sink_set_callbacks(sink_, &cbs, this, nullptr);
    spdlog::info("[AI_SERVICE] settings complete");
}

void AiService::detach() {
    if (sink_) {
        gst_app_sink_set_callbacks(sink_, nullptr, nullptr, nullptr);
        gst_object_unref(sink_);
        sink_ = nullptr;
    }
}

GstFlowReturn AiService::on_new_sample(GstAppSink* sink, gpointer user_data) {
    auto* self = static_cast<AiService*>(user_data);
    (void)self;

    GstSample* sample = gst_app_sink_pull_sample(sink);
    if (!sample) return GST_FLOW_ERROR;

    // 1) 캡스/비디오정보
    GstCaps* caps = gst_sample_get_caps(sample);
    GstVideoInfo vinfo;
    bool has_vinfo = (caps && gst_video_info_from_caps(&vinfo, caps));

    if (has_vinfo) {
        const GstStructure* s = gst_caps_get_structure(caps, 0);
        int w = GST_VIDEO_INFO_WIDTH(&vinfo);
        int h = GST_VIDEO_INFO_HEIGHT(&vinfo);
        GstVideoFormat fmt = GST_VIDEO_INFO_FORMAT(&vinfo);

        // FPS 가져오기 (caps의 framerate fraction)
        int fps_n = 0, fps_d = 1;
        gst_structure_get_fraction(s, "framerate", &fps_n, &fps_d);

        spdlog::info("[caps] format={} (I420? {}), {}x{}, fps={}/{}",
                      gst_structure_has_field_typed(s, "format", G_TYPE_STRING) ?
                        gst_structure_get_string(s, "format") : "unknown",
                      (fmt == GST_VIDEO_FORMAT_I420), w, h, fps_n, fps_d);
    } else {
        spdlog::info("[caps] (no/invalid caps)");
    }

    // 2) 버퍼 메타
    GstBuffer* buf = gst_sample_get_buffer(sample);
    if (!buf) { gst_sample_unref(sample); return GST_FLOW_ERROR; }

    // 추론 코드
//    NvDsBatchMeta* batch_meta = gst_buffer_get_nvds_batch_meta(buf);
//    if (batch_meta) {
//        // 프레임 메타데이터 리스트를 순회
//        for (NvDsMetaList* l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) {
//            NvDsFrameMeta* frame_meta = (NvDsFrameMeta*)l_frame->data;
//            // 객체 메타데이터 리스트를 순회
//            for (NvDsMetaList* l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next) {
//                NvDsObjectMeta* obj_meta = (NvDsObjectMeta*)l_obj->data;
//                // 여기서 obj_meta->rect_params (바운딩 박스), obj_meta->class_id 등으로 추론 결과에 접근
//                spdlog::info("Found object with class_id: {}, confidence: {}", obj_meta->class_id, obj_meta->confidence);
//            }
//        }
//    }

    guint nmem = gst_buffer_n_memory(buf);
    GstClockTime pts = GST_BUFFER_PTS(buf);
    GstClockTime dts = GST_BUFFER_DTS(buf);
    GstClockTime dur = GST_BUFFER_DURATION(buf);

    spdlog::info("[buffer] nmem={}, PTS={} ns, DTS={} ns, DUR={} ns, flags=0x{:x}",
                  nmem, (guint64)pts, (guint64)dts, (guint64)dur, (unsigned)GST_BUFFER_FLAGS(buf));

    // 3) 맵해서 크기/데이터 접근
    GstMapInfo map;
    if (gst_buffer_map(buf, &map, GST_MAP_READ)) {
        spdlog::info("[map] size={} bytes, data_ptr={}", (gsize)map.size, (const void*)map.data);
        gst_buffer_unmap(buf, &map);
    } else {
        spdlog::warn("[map] failed");
    }

    // 4) 비디오 메타(plane pitch/offset 등)
    if (has_vinfo) {
        // I420은 3 planes: Y, U, V
        int n_planes = GST_VIDEO_INFO_N_PLANES(&vinfo);
        for (int i = 0; i < n_planes; ++i) {
            spdlog::info("[video] plane{}: stride={}, offset={}, plane_height={}",
                          i,
                          GST_VIDEO_INFO_PLANE_STRIDE(&vinfo, i),
                          GST_VIDEO_INFO_PLANE_OFFSET(&vinfo, i),
                          GST_VIDEO_INFO_COMP_HEIGHT(&vinfo, i));
        }
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}
