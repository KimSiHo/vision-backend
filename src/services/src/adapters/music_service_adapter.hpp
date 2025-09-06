#pragma once

#include <nlohmann/json.hpp>

#include "services/music_service.hpp"
#include "adapters/i_service.hpp"

class MusicServiceAdapter : public IService {
public:
    explicit MusicServiceAdapter(MusicService& svc);

    bool handle(const std::string& cmd, nlohmann::json& reply) override;

private:
    MusicService& svc_;
};
