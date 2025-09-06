#include "adapters/music_service_adapter.hpp"

MusicServiceAdapter::MusicServiceAdapter(MusicService& svc)
    : svc_(svc) {}

bool MusicServiceAdapter::handle(const std::string& cmd, nlohmann::json& reply) {
    if (cmd == "MUSIC_PLAY") {
        svc_.play();
        reply = {{"ok", true}, {"msg", "music play"}};
        return true;
    }
    if (cmd == "MUSIC_STOP") {
        svc_.stop();
        reply = {{"ok", true}, {"msg", "music stop"}};
        return true;
    }
    if (cmd == "MUSIC_PAUSE") {
        svc_.pause();
        reply = {{"ok", true}, {"msg", "music pause"}};
        return true;
    }
    if (cmd == "MUSIC_NEXT") {
        svc_.next();
        reply = {{"ok", true}, {"msg", "music next"}};
        return true;
    }
    if (cmd == "MUSIC_PREV") {
        svc_.prev();
        reply = {{"ok", true}, {"msg", "music prev"}};
        return true;
    }
    return false;
}
