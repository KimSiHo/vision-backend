#include "impl/music/custom-pipeline/source_bin.hpp"

#include <glib-object.h>

#include "common/utils/logging.hpp"

void printSignals(GstElement* element) {
  GType type = G_OBJECT_TYPE(element);
  guint n_ids = 0;
  guint* ids = g_signal_list_ids(type, &n_ids);

  SPDLOG_SERVICE_INFO("Element '{}' has {} signals", GST_ELEMENT_NAME(element), n_ids);

  for (guint i = 0; i < n_ids; i++) {
    GSignalQuery query;
    g_signal_query(ids[i], &query);

    SPDLOG_SERVICE_INFO("  signal: {} (id={})", query.signal_name, ids[i]);
  }

  g_free(ids);
}

GstElement* createSourceBin() {
  GstElement* bin = gst_bin_new("source-bin");
  GstElement* filesrc = gst_element_factory_make("filesrc", "file-source");
  GstElement* id3 = gst_element_factory_make("id3demux", "id3demux");
  printSignals(id3);

  GstElement* parse = gst_element_factory_make("mpegaudioparse", "mpegaudioparse");
  GstElement* dec = gst_element_factory_make("mpg123audiodec", "mp3-decoder");
  GstElement* queue = gst_element_factory_make("queue", "src-queue");

  if (!bin || !filesrc || !id3 || !parse || !dec || !queue) {
    SPDLOG_SERVICE_ERROR("[SourceBin] Failed to create elements");
    return nullptr;
  }

  gst_bin_add_many(GST_BIN(bin), filesrc, id3, parse, dec, queue, nullptr);

  if (!gst_element_link(filesrc, id3)) SPDLOG_SERVICE_ERROR("[SourceBin] Failed to link filesrc → id3demux");
  if (!gst_element_link(id3, parse)) SPDLOG_SERVICE_ERROR("[SourceBin] Failed to link id3demux → mpegaudioparse");
  if (!gst_element_link_many(parse, dec, queue, nullptr))
    SPDLOG_SERVICE_ERROR("[SourceBin] Failed to link parse → dec → queue");

  // ghost pad 노출
  GstPad* qsrc = gst_element_get_static_pad(queue, "src");
  GstPad* ghost = gst_ghost_pad_new("src", qsrc);
  gst_pad_set_active(ghost, TRUE);
  gst_element_add_pad(bin, ghost);
  gst_object_unref(qsrc);

  return bin;
}
