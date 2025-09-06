#pragma once

#include "adapters/i_service.hpp"
#include "common/utils/json.hpp"
#include "services/audio/audio_service.hpp"

class AudioServiceAdapter : public IService {
public:
  explicit AudioServiceAdapter(AudioService& service);

  bool handle(const std::string& command, app_common::Json& reply) override;

private:
  AudioService& service_;
};
