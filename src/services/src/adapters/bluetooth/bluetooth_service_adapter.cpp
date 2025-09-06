#include "adapters/bluetooth/bluetooth_service_adapter.hpp"

BluetoothServiceAdapter::BluetoothServiceAdapter(BluetoothService& service) : service_(service) {}

bool BluetoothServiceAdapter::handle(const std::string& command, app_common::Json& reply) {
  if (command == "BT_ON") {
    service_.powerOn();
    reply = {{"ok", true}, {"msg", "bluetooth on"}};
    return true;
  }

  if (command == "BT_OFF") {
    service_.powerOff();
    reply = {{"ok", true}, {"msg", "bluetooth off"}};
    return true;
  }

  if (command == "BT_SCAN") {
    service_.scan();
    reply = {{"ok", true}, {"msg", "bluetooth scanning"}};
    return true;
  }

  if (command.rfind("BT_CONNECT:", 0) == 0) {
    auto device = command.substr(std::string("BT_CONNECT:").size());
    service_.connect(device);
    reply = {{"ok", true}, {"msg", "bluetooth connecting"}, {"device", device}};
    return true;
  }

  return false;
}
