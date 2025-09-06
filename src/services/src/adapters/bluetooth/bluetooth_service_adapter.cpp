#include "adapters/bluetooth/bluetooth_service_adapter.hpp"

BluetoothServiceAdapter::BluetoothServiceAdapter(BluetoothService& svc)
    : svc_(svc) {}

bool BluetoothServiceAdapter::handle(const std::string& cmd, AppCommon::json& reply) {
    if (cmd == "BT_ON") {
        svc_.power_on();
        reply = {{"ok", true}, {"msg", "bluetooth on"}};
        return true;
    }
    if (cmd == "BT_OFF") {
        svc_.power_off();
        reply = {{"ok", true}, {"msg", "bluetooth off"}};
        return true;
    }
    if (cmd == "BT_SCAN") {
        svc_.scan();
        reply = {{"ok", true}, {"msg", "bluetooth scanning"}};
        return true;
    }
    if (cmd.rfind("BT_CONNECT:", 0) == 0) {
        auto device = cmd.substr(std::string("BT_CONNECT:").size());
        svc_.connect(device);
        reply = {{"ok", true}, {"msg", "bluetooth connecting", {"device", device}}};
        return true;
    }
    return false;
}
