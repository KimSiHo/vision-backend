#pragma once

#include <string>

#include "common/utils/json.hpp"

class IService {
public:
    virtual bool handle(const std::string& cmd, AppCommon::json& reply) = 0;
    virtual ~IService() = default;
};
