#pragma once

#include "services/music/music_service.hpp"
#include "adapters/i_service.hpp"
#include "common/utils/json.hpp"

class MusicServiceAdapter : public IService {
public:
    explicit MusicServiceAdapter(MusicService& svc);

    bool handle(const std::string& cmd, AppCommon::json& reply) override;

private:
    MusicService& svc_;
};
