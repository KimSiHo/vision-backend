#include "adapters/music/music_service_adapter.hpp"

MusicServiceAdapter::MusicServiceAdapter(MusicService& service) : service_(service) {}

bool MusicServiceAdapter::handle(const std::string& command, app_common::Json& reply) {
  if (command == "MUSIC_PLAY") {
    service_.play();
    reply = {{"ok", true}, {"msg", "music play"}};
    return true;
  }

  if (command == "MUSIC_STOP") {
    service_.stop();
    reply = {{"ok", true}, {"msg", "music stop"}};
    return true;
  }

  if (command == "MUSIC_PAUSE") {
    service_.pause();
    reply = {{"ok", true}, {"msg", "music pause"}};
    return true;
  }

  if (command == "MUSIC_NEXT") {
    service_.next();
    reply = {{"ok", true}, {"msg", "music next"}};
    return true;
  }

  if (command == "MUSIC_PREV") {
    service_.prev();
    reply = {{"ok", true}, {"msg", "music prev"}};
    return true;
  }

  return false;
}
