#pragma once

#include "common/utils/json.hpp"
#include "services/audio/audio_service.hpp"
#include "adapters/i_service.hpp"

class AudioServiceAdapter : public IService {
public:
    explicit AudioServiceAdapter(AudioService& svc);

    bool handle(const std::string& cmd, AppCommon::json& reply) override;

private:
    AudioService& svc_;
};
