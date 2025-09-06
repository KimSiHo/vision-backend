#pragma once

#include "adapters/i_service.hpp"
#include "common/utils/json.hpp"
#include "services/bluetooth/bluetooth_service.hpp"

class BluetoothServiceAdapter : public IService {
public:
  explicit BluetoothServiceAdapter(BluetoothService& service);

  bool handle(const std::string& command, app_common::Json& reply) override;

private:
  BluetoothService& service_;
};
