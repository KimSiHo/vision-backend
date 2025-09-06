#include "adapters/audio/audio_service_adapter.hpp"

AudioServiceAdapter::AudioServiceAdapter(AudioService& service) : service_(service) {}

bool AudioServiceAdapter::handle(const std::string& command, app_common::Json& reply) {
  if (command == "AUDIO_VOL_UP") {
    return true;
  }

  if (command == "AUDIO_VOL_DOWN") {
    return true;
  }

  if (command.rfind("AUDIO_VOL_SET", 0) == 0) {
    auto pos = command.find(':');
    if (pos != std::string::npos) {
      int level = std::stoi(command.substr(pos + 1));
      service_.setVolume(level);

      auto volume = service_.getVolume();
      reply = {{"ok", true}, {"msg", "volume set"}, {"volume", volume.has_value() ? volume.value() : -1}};
      return true;
    }
  }

  return false;
}
