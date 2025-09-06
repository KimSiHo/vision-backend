#pragma once

#include "services/camera/camera_service.hpp"
#include "adapters/i_service.hpp"
#include "common/utils/json.hpp"

class CameraServiceAdapter : public IService {
public:
    explicit CameraServiceAdapter(CameraService& svc);

    bool handle(const std::string& cmd, AppCommon::json& reply) override;

private:
    CameraService& svc_;
};
