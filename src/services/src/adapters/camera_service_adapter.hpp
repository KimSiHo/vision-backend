#pragma once

#include <nlohmann/json.hpp>
#include "services/camera_service.hpp"
#include "adapters/i_service.hpp"

class CameraServiceAdapter : public IService {
public:
    explicit CameraServiceAdapter(CameraService& svc);

    bool handle(const std::string& cmd, nlohmann::json& reply) override;

private:
    CameraService& svc_;
};
