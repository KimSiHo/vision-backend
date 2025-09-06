#pragma once

#include <string>

#include <nlohmann/json.hpp>

class IService {
public:
    virtual bool handle(const std::string& cmd, nlohmann::json& reply) = 0;
    virtual ~IService() = default;
};
