#pragma once

#include "adapters/i_service.hpp"
#include "common/utils/json.hpp"
#include "services/music/music_service.hpp"

class MusicServiceAdapter : public IService {
public:
  explicit MusicServiceAdapter(MusicService& service);

  bool handle(const std::string& command, app_common::Json& reply) override;

private:
  MusicService& service_;
};
