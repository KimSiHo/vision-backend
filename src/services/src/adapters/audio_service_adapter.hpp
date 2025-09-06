#pragma once

#include "common/json.hpp"
#include "services/audio_service.hpp"
#include "adapters/i_service.hpp"

class AudioServiceAdapter : public IService {
public:
    explicit AudioServiceAdapter(AudioService& svc);

    bool handle(const std::string& cmd, common::json& reply) override;

private:
    AudioService& svc_;
};
