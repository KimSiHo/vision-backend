#include "impl/music/custom-pipeline/sink_bin.hpp"

#include "common/utils/logging.hpp"

GstElement* createSinkBin() {
  GstElement* bin = gst_bin_new("sink-bin");
  GstElement* convert = gst_element_factory_make("audioconvert", "convert");
  GstElement* resample = gst_element_factory_make("audioresample", "resample");
  GstElement* sink = gst_element_factory_make("pulsesink", "sink");

  if (!bin || !convert || !resample || !sink) {
    SPDLOG_SERVICE_ERROR("[AudioBin] Failed to create elements");
    return nullptr;
  }

  g_object_set(sink, "enable-last-sample", FALSE, nullptr);

  gst_bin_add_many(GST_BIN(bin), convert, resample, sink, nullptr);

  if (!gst_element_link_many(convert, resample, sink, nullptr))
    SPDLOG_SERVICE_ERROR("[AudioBin] Failed to link convert → resample → sink");

  // ghost pad 노출
  GstPad* csink = gst_element_get_static_pad(convert, "sink");
  GstPad* ghost = gst_ghost_pad_new("sink", csink);
  gst_pad_set_active(ghost, TRUE);
  gst_element_add_pad(bin, ghost);
  gst_object_unref(csink);

  return bin;
}
