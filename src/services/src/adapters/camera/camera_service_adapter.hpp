#pragma once

#include "adapters/i_service.hpp"
#include "common/utils/json.hpp"
#include "services/camera/camera_service.hpp"

class CameraServiceAdapter : public IService {
public:
  explicit CameraServiceAdapter(CameraService& service);

  bool handle(const std::string& command, app_common::Json& reply) override;

private:
  CameraService& service_;
};
