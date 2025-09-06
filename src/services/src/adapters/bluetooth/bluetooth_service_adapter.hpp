#pragma once

#include "common/utils/json.hpp"
#include "services/bluetooth/bluetooth_service.hpp"
#include "adapters/i_service.hpp"

class BluetoothServiceAdapter : public IService {
public:
    explicit BluetoothServiceAdapter(BluetoothService& svc);

    bool handle(const std::string& cmd, AppCommon::json& reply) override;

private:
    BluetoothService& svc_;
};
