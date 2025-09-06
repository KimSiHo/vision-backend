#include "services/infer/ai_service.hpp"

#include <gst/video/video.h>
#include <gstnvdsmeta.h>

#include <chrono>

#include "common/utils/json.hpp"
#include "common/utils/logging.hpp"
#include "config/zmq_config.hpp"

AiService::AiService(GstElement* appsink_elem, PubSocket& pub_socket) : pub_socket_(pub_socket) {
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
  SPDLOG_SERVICE_INFO("[AI] Service ready (publishing results)");
  int job;
  app_common::Json inference_result;

  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void AiService::attach(GstElement* appsink_elem) {
  detach();
  if (!appsink_elem) return;

  sink_ = GST_APP_SINK(gst_object_ref(appsink_elem));

  GstAppSinkCallbacks cbs{};
  cbs.new_sample = &AiService::onNewSample;
  gst_app_sink_set_callbacks(sink_, &cbs, this, nullptr);
  SPDLOG_SERVICE_INFO("[AI_SERVICE] settings complete");
}

void AiService::detach() {
  if (sink_) {
    gst_app_sink_set_callbacks(sink_, nullptr, nullptr, nullptr);
    gst_object_unref(sink_);
    sink_ = nullptr;
  }
}

GstFlowReturn AiService::onNewSample(GstAppSink* sink, gpointer user_data) {
  auto* self = static_cast<AiService*>(user_data);

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

    //        SPDLOG_SERVICE_INFO("[caps] format={} (I420? {}), {}x{}, fps={}/{}",
    //                      gst_structure_has_field_typed(s, "format", G_TYPE_STRING) ?
    //                        gst_structure_get_string(s, "format") : "unknown",
    //                      (fmt == GST_VIDEO_FORMAT_I420), w, h, fps_n, fps_d);
  } else {
    SPDLOG_SERVICE_INFO("[caps] (no/invalid caps)");
  }

  // 2) 버퍼 메타
  GstBuffer* buf = gst_sample_get_buffer(sample);
  if (!buf) {
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  // 추론 코드
  NvDsBatchMeta* batch_meta = gst_buffer_get_nvds_batch_meta(buf);
  if (batch_meta) {
    // 프레임 메타데이터 리스트를 순회
    for (NvDsMetaList* l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) {
      NvDsFrameMeta* frame_meta = (NvDsFrameMeta*)l_frame->data;

      // 이 프레임에 대한 JSON 객체 생성 준비
      app_common::Json frame_json;
      frame_json["frame_number"] = frame_meta->frame_num;
      frame_json["timestamp"] = frame_meta->buf_pts;

      // 검출된 객체들을 담을 JSON 배열
      app_common::Json objects_array = app_common::Json::array();

      // 객체 메타데이터 리스트를 순회
      for (NvDsMetaList* l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next) {
        NvDsObjectMeta* obj_meta = (NvDsObjectMeta*)l_obj->data;

        // 객체 하나에 대한 정보를 담을 JSON 객체
        app_common::Json object_json;
        object_json["class_id"] = obj_meta->class_id;
        object_json["label"] = std::string(obj_meta->obj_label);
        object_json["confidence"] = obj_meta->confidence;

        // 바운딩 박스 좌표 정보를 담을 JSON 객체
        app_common::Json box_json;
        box_json["x"] = obj_meta->rect_params.left;
        box_json["y"] = obj_meta->rect_params.top;
        box_json["w"] = obj_meta->rect_params.width;
        box_json["h"] = obj_meta->rect_params.height;

        object_json["box"] = box_json;

        // 생성된 객체 JSON을 배열에 추가
        objects_array.push_back(object_json);
      }

      // 완성된 객체 배열을 프레임 JSON에 추가
      frame_json["objects"] = objects_array;

      // 완성된 JSON을 문자열로 변환하여 ZMQ로 전송
      std::string json_string_to_send = frame_json.dump();
      self->pub_socket_.publish(std::string(app_config::kTopicDetections), json_string_to_send);

      SPDLOG_SERVICE_DEBUG("Sending JSON: {}", json_string_to_send);
    }
  }

  guint nmem = gst_buffer_n_memory(buf);
  GstClockTime pts = GST_BUFFER_PTS(buf);
  GstClockTime dts = GST_BUFFER_DTS(buf);
  GstClockTime dur = GST_BUFFER_DURATION(buf);

  //    SPDLOG_SERVICE_INFO("[buffer] nmem={}, PTS={} ns, DTS={} ns, DUR={} ns, flags=0x{:x}",
  //                  nmem, (guint64)pts, (guint64)dts, (guint64)dur, (unsigned)GST_BUFFER_FLAGS(buf));

  // 3) 맵해서 크기/데이터 접근
  GstMapInfo map;
  if (gst_buffer_map(buf, &map, GST_MAP_READ)) {
    //        SPDLOG_SERVICE_INFO("[map] size={} bytes, data_ptr={}", (gsize)map.size, (const void*)map.data);
    gst_buffer_unmap(buf, &map);
  } else {
    SPDLOG_SERVICE_WARN("[map] failed");
  }

  // 4) 비디오 메타(plane pitch/offset 등)
  if (has_vinfo) {
    // I420은 3 planes: Y, U, V
    int n_planes = GST_VIDEO_INFO_N_PLANES(&vinfo);
    for (int i = 0; i < n_planes; ++i) {
      //            SPDLOG_SERVICE_INFO("[video] plane{}: stride={}, offset={}, plane_height={}",
      //                          i,
      //                          GST_VIDEO_INFO_PLANE_STRIDE(&vinfo, i),
      //                          GST_VIDEO_INFO_PLANE_OFFSET(&vinfo, i),
      //                          GST_VIDEO_INFO_COMP_HEIGHT(&vinfo, i));
    }
  }

  gst_sample_unref(sample);
  return GST_FLOW_OK;
}
