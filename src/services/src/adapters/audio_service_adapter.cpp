#include "adapters/audio_service_adapter.hpp"

AudioServiceAdapter::AudioServiceAdapter(AudioService& svc)
    : svc_(svc) {}

bool AudioServiceAdapter::handle(const std::string& cmd, common::json& reply) {
    if (cmd == "AUDIO_VOL_UP") {
        return true;
    }
    if (cmd == "AUDIO_VOL_DOWN") {
        return true;
    }
    if (cmd.rfind("AUDIO_VOL_SET", 0) == 0) {
        auto pos = cmd.find(":");
        if (pos != std::string::npos) {
            int level = std::stoi(cmd.substr(pos + 1));
            svc_.set_volume(level);

            auto vol = svc_.get_volume();
            reply = {
                {"ok", true},
                {"msg", "volume set"},
                {"volume", vol.has_value() ? vol.value() : -1} // 없으면 -1
            };
            return true;
        }
    }
    return false;
}
