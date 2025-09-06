#pragma once

#include <string>

#include "common/utils/json.hpp"

class IService {
public:
  virtual bool handle(const std::string& command, app_common::Json& reply) = 0;
  virtual ~IService() = default;
};
