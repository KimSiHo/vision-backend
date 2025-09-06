#include "services/audio_service.hpp"

#include <pulse/pulseaudio.h>
#include <spdlog/spdlog.h>

AudioService::AudioService() : sink_("@DEFAULT_SINK@") {}

AudioService::~AudioService() = default;

void AudioService::set_sink(const std::string& sink_name) {
    sink_ = sink_name;
    spdlog::info("[Audio] sink set to {}", sink_);
}

void AudioService::set_volume(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    pa_mainloop* ml = pa_mainloop_new();
    pa_mainloop_api* api = pa_mainloop_get_api(ml);
    pa_context* ctx = pa_context_new(api, "AudioService");

    pa_context_connect(ctx, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
    pa_mainloop_iterate(ml, 1, nullptr);  // 연결 시도

    pa_cvolume vol;
    pa_cvolume_set(&vol, 2, (pa_volume_t)(percent * PA_VOLUME_NORM / 100));

    pa_operation* op = pa_context_set_sink_volume_by_name(
        ctx, sink_.c_str(), &vol, nullptr, nullptr
    );

    if (op) {
        pa_operation_unref(op);
        spdlog::info("[Audio] Volume set to {}%", percent);
    } else {
        spdlog::error("[Audio] Failed to set volume for sink {}", sink_);
    }

    pa_context_disconnect(ctx);
    pa_context_unref(ctx);
    pa_mainloop_free(ml);
}

std::optional<int> AudioService::get_volume() const {
    // 간단 구현: PulseAudio API에서 비동기로 sink info 가져와야 함.
    // 여기서는 TODO 처리.
    spdlog::warn("[Audio] get_volume() not yet implemented with libpulse API");
    return std::nullopt;
}
