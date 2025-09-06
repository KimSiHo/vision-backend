#pragma once

#include "common/json.hpp"
#include "services/bluetooth_service.hpp"
#include "adapters/i_service.hpp"

class BluetoothServiceAdapter : public IService {
public:
    explicit BluetoothServiceAdapter(BluetoothService& svc);

    bool handle(const std::string& cmd, common::json& reply) override;

private:
    BluetoothService& svc_;
};
