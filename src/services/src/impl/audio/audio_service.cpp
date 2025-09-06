#include "services/audio/audio_service.hpp"

#include "common/utils/logging.hpp"

namespace {
    struct VolumeQuery {
        std::optional<int> volume;
        bool done = false;
    };
}

AudioService::AudioService() : sink_("@DEFAULT_SINK@"), ml_(nullptr), api_(nullptr), ctx_(nullptr) {
    ml_ = pa_mainloop_new();
    if (!ml_) {
        SPDLOG_SERVICE_ERROR("Failed to create PulseAudio mainloop");
        return;
    }

    api_ = pa_mainloop_get_api(ml_);
    ctx_ = pa_context_new(api_, "AudioService");
    if (!ctx_) {
        SPDLOG_SERVICE_ERROR("Failed to create PulseAudio context");
        pa_mainloop_free(ml_);
        ml_ = nullptr;
        return;
    }

    if (pa_context_connect(ctx_, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        SPDLOG_SERVICE_ERROR("Failed to connect PulseAudio context: {}", pa_strerror(pa_context_errno(ctx_)));
        pa_context_unref(ctx_);
        pa_mainloop_free(ml_);
        ctx_ = nullptr;
        ml_ = nullptr;
        return;
    }

    while (true) {
        pa_mainloop_iterate(ml_, 1, nullptr);
        auto state = pa_context_get_state(ctx_);
        if (state == PA_CONTEXT_READY) break;
        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
            SPDLOG_SERVICE_ERROR("Context connection failed/terminated");
            return;
        }
    }

    pa_operation* op = pa_context_get_server_info(ctx_, serverInfoCb, this);
    if (op) {
        while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
            pa_mainloop_iterate(ml_, 1, nullptr);
        }
        pa_operation_unref(op);
    }

    SPDLOG_SERVICE_INFO("PulseAudio context initialized (sink={})", sink_);
}

AudioService::~AudioService() {
    if (ctx_) {
        pa_context_disconnect(ctx_);
        pa_context_unref(ctx_);
        ctx_ = nullptr;
    }
    if (ml_) {
        pa_mainloop_free(ml_);
        ml_ = nullptr;
    }

    SPDLOG_SERVICE_INFO("PulseAudio resources cleaned up");
}

void AudioService::setSink(const std::string& sink_name) {
    sink_ = sink_name;
}

const std::string& AudioService::getSink() const {
    return sink_;
}

void AudioService::setVolume(int percent) {
    if (!ctx_) {
        SPDLOG_SERVICE_ERROR("Context not initialized");
        return;
    }

    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    pa_cvolume vol;
    pa_cvolume_set(&vol, 2, (pa_volume_t)(percent * PA_VOLUME_NORM / 100));

    pa_operation* op = pa_context_set_sink_volume_by_name(ctx_, sink_.c_str(), &vol, nullptr, nullptr);

    if (op) {
        pa_operation_unref(op);
        SPDLOG_SERVICE_DEBUG("Volume set to {}%", percent);
    } else {
        SPDLOG_SERVICE_ERROR("Failed to set volume for sink {}", sink_);
    }

    pa_mainloop_iterate(ml_, 1, nullptr);
}

std::optional<int> AudioService::getVolume() const {
    if (!ctx_ || !ml_) {
        SPDLOG_SERVICE_ERROR("getVolume() called but PulseAudio context not initialized");
        return std::nullopt;
    }

    VolumeQuery query;

    pa_operation* op = pa_context_get_sink_info_by_name(ctx_, sink_.c_str(), sinkInfoCb, &query);

    if (!op) {
        SPDLOG_SERVICE_ERROR("Failed to request sink info for {}", sink_);
        return std::nullopt;
    }

    while (!query.done) {
        pa_mainloop_iterate(ml_, 1, nullptr);
    }

    pa_operation_unref(op);

    if (query.volume) {
        SPDLOG_SERVICE_DEBUG("Current volume of {} = {}%", sink_, *query.volume);
    } else {
        SPDLOG_SERVICE_WARN("Could not read volume of sink {}", sink_);
    }

    return query.volume;
}

void AudioService::serverInfoCb(pa_context*, const pa_server_info* i, void* userdata) {
    SPDLOG_SERVICE_INFO(R"(Server name: {}, Server version: {}, Default sink: {}, Default source: {})",
        i->server_name, i->server_version, i->default_sink_name, i->default_source_name
    );

    if (!i || !i->default_sink_name) return;

    auto* self = static_cast<AudioService*>(userdata);
    self->setSink(i->default_sink_name);

    SPDLOG_SERVICE_INFO("Default sink detected: {}", self->getSink());
}

void AudioService::sinkInfoCb(pa_context*, const pa_sink_info* i, int eol, void* userdata) {
    if (eol > 0 || !i) return;

    auto* query = static_cast<VolumeQuery*>(userdata);
    if (i->volume.channels > 0) {
        // 평균 볼륨을 percent로 계산
        pa_volume_t avg = pa_cvolume_avg(&i->volume);
        int percent = static_cast<int>((avg * 100) / PA_VOLUME_NORM);
        query->volume = percent;
    }

    query->done = true;
}
