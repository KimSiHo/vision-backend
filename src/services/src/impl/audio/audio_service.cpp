#include "services/audio/audio_service.hpp"

#include "common/utils/logging.hpp"

namespace {
struct VolumeQuery {
  std::optional<int> volume;
  bool done = false;
};
}  // namespace

AudioService::AudioService() : sink_("@DEFAULT_SINK@"), mainloop_(nullptr), mainloop_api_(nullptr), context_(nullptr) {
  mainloop_ = pa_mainloop_new();
  if (!mainloop_) {
    // SPDLOG_SERVICE_ERROR("Failed to create PulseAudio mainloop");
    return;
  }

  mainloop_api_ = pa_mainloop_get_api(mainloop_);
  context_ = pa_context_new(mainloop_api_, "AudioService");
  if (!context_) {
    // SPDLOG_SERVICE_ERROR("Failed to create PulseAudio context");
    pa_mainloop_free(mainloop_);
    mainloop_ = nullptr;
    return;
  }

  if (pa_context_connect(context_, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
    // SPDLOG_SERVICE_ERROR("Failed to connect PulseAudio context: {}", pa_strerror(pa_context_errno(context_)));
    pa_context_unref(context_);
    pa_mainloop_free(mainloop_);
    context_ = nullptr;
    mainloop_ = nullptr;
    return;
  }

  while (true) {
    pa_mainloop_iterate(mainloop_, 1, nullptr);
    auto state = pa_context_get_state(context_);
    if (state == PA_CONTEXT_READY) break;
    if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
      // SPDLOG_SERVICE_ERROR("Context connection failed/terminated");
      return;
    }
  }

  pa_operation* op = pa_context_get_server_info(context_, serverInfoCb, this);
  if (op) {
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
      pa_mainloop_iterate(mainloop_, 1, nullptr);
    }
    pa_operation_unref(op);
  }

  // SPDLOG_SERVICE_INFO("PulseAudio context initialized (sink={})", sink_);
}

AudioService::~AudioService() {
  if (context_) {
    pa_context_disconnect(context_);
    pa_context_unref(context_);
    context_ = nullptr;
  }
  if (mainloop_) {
    pa_mainloop_free(mainloop_);
    mainloop_ = nullptr;
  }

  // SPDLOG_SERVICE_INFO("PulseAudio resources cleaned up");
}

void AudioService::setSink(const std::string& sink_name) { sink_ = sink_name; }

const std::string& AudioService::getSink() const { return sink_; }

void AudioService::setVolume(int percent) {
  if (!context_) {
    // SPDLOG_SERVICE_ERROR("Context not initialized");
    return;
  }

  percent = std::clamp(percent, 0, 100);

  pa_cvolume volume;
  pa_cvolume_set(&volume, 2, static_cast<pa_volume_t>(percent * PA_VOLUME_NORM / 100));

  pa_operation* op = pa_context_set_sink_volume_by_name(context_, sink_.c_str(), &volume, nullptr, nullptr);

  if (op) {
    pa_operation_unref(op);
    // SPDLOG_SERVICE_DEBUG("Volume set to {}%", percent);
  } else {
    // SPDLOG_SERVICE_ERROR("Failed to set volume for sink {}", sink_);
  }

  pa_mainloop_iterate(mainloop_, 1, nullptr);
}

std::optional<int> AudioService::getVolume() const {
  if (!context_ || !mainloop_) {
    // SPDLOG_SERVICE_ERROR("getVolume() called but PulseAudio context not initialized");
    return std::nullopt;
  }

  VolumeQuery query;
  pa_operation* op = pa_context_get_sink_info_by_name(context_, sink_.c_str(), sinkInfoCb, &query);

  if (!op) {
    // SPDLOG_SERVICE_ERROR("Failed to request sink info for {}", sink_);
    return std::nullopt;
  }

  while (!query.done) {
    pa_mainloop_iterate(mainloop_, 1, nullptr);
  }

  pa_operation_unref(op);

  if (query.volume) {
    // SPDLOG_SERVICE_DEBUG("Current volume of {} = {}%", sink_, *query.volume);
  } else {
    // SPDLOG_SERVICE_WARN("Could not read volume of sink {}", sink_);
  }

  return query.volume;
}

void AudioService::serverInfoCb(pa_context*, const pa_server_info* info, void* userdata) {
  if (!info || !info->default_sink_name) return;

  auto* self = static_cast<AudioService*>(userdata);
  self->setSink(info->default_sink_name);

  // SPDLOG_SERVICE_INFO("Server: {}, Version: {}, Default sink: {}, Default source: {}", info->server_name,
  //                      info->server_version, info->default_sink_name, info->default_source_name);
  //  SPDLOG_SERVICE_INFO("Default sink detected: {}", self->getSink());
}

void AudioService::sinkInfoCb(pa_context*, const pa_sink_info* info, int eol, void* userdata) {
  if (eol > 0 || !info) return;

  auto* query = static_cast<VolumeQuery*>(userdata);
  if (info->volume.channels > 0) {
    pa_volume_t avg = pa_cvolume_avg(&info->volume);
    query->volume = static_cast<int>((avg * 100) / PA_VOLUME_NORM);
  }

  query->done = true;
}
