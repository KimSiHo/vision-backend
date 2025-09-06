#include "services/bluetooth_service.hpp"

#include <sdbus-c++/sdbus-c++.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "config/app_config.hpp"

BluetoothService::BluetoothService()
    : pub_socket_(AppConfig::context(), zmq::socket_type::pub)
{
    try {
        pub_socket_.bind("ipc:///tmp/bt_scan.sock");
        spdlog::info("[BT] PUB socket bound at ipc:///tmp/bt_scan.sock");
    } catch (const zmq::error_t& e) {
        spdlog::error("[BT] Failed to bind PUB socket: {}", e.what());
    }
}

void BluetoothService::power_on() {
    spdlog::info("[BT] Power ON");

    auto bus = sdbus::createSystemBusConnection();

    const char* adapterPath = "/org/bluez/hci0";
    auto adapter = sdbus::createProxy(*bus, "org.bluez", adapterPath);

    adapter->callMethod("Set")
        .onInterface("org.freedesktop.DBus.Properties")
        .withArguments("org.bluez.Adapter1", "Powered", sdbus::Variant(true));

    spdlog::info("[BT] Power ON Success");
}

void BluetoothService::power_off() {
    spdlog::info("[BT] Power OFF");

    auto bus = sdbus::createSystemBusConnection();

    const char* adapterPath = "/org/bluez/hci0";
    auto adapter = sdbus::createProxy(*bus, "org.bluez", adapterPath);

    adapter->callMethod("Set")
        .onInterface("org.freedesktop.DBus.Properties")
        .withArguments("org.bluez.Adapter1", "Powered", sdbus::Variant(false));

    spdlog::info("[BT] Power OFF Success");
}

void BluetoothService::scan() {
    spdlog::info("[BT] Scanning...");

    // 1) DBus 연결
    bus_ = sdbus::createSystemBusConnection();
    om_proxy_ = sdbus::createProxy(*bus_, "org.bluez", "/");

    // 2) InterfacesAdded 시그널 핸들러 등록
    om_proxy_->uponSignal("InterfacesAdded")
        .onInterface("org.freedesktop.DBus.ObjectManager")
        .call([this](const sdbus::ObjectPath& path,
                     const std::map<std::string, std::map<std::string, sdbus::Variant>>& ifaces) {
            spdlog::info(">>>>>>>>>>>>>>>>>>>>>>>>>>");
            spdlog::info("[BT] InterfacesAdded for path: {}", path.c_str());

            auto it = ifaces.find("org.bluez.Device1");
            if (it != ifaces.end()) {
                const auto& props = it->second;

                std::string addr, name;
                if (auto p = props.find("Address"); p != props.end()) {
                    addr = p->second.get<std::string>();
                }
                if (auto p = props.find("Name"); p != props.end()) {
                    name = p->second.get<std::string>();
                }

                if (!addr.empty()) {
                    devices_[addr] = name;
                    spdlog::info("[BT] Added device: {} ({})", addr, name);
                    publish_scan_result(build_device_list_json());
                }
            }
        });

    // 3) InterfacesRemoved 시그널 핸들러 등록
  // om_proxy_->uponSignal("InterfacesRemoved")
  //     .onInterface("org.freedesktop.DBus.ObjectManager")
  //     .call([this](const sdbus::ObjectPath& path,
  //                  const std::vector<std::string>& ifaces) {
  //         spdlog::info("[BT] InterfacesRemoved: {}", path.c_str());
//
  //         std::string devPath = path;
  //         auto pos = devPath.find("dev_");
  //         if (pos != std::string::npos) {
  //             std::string devAddr = devPath.substr(pos + 4); // "XX_XX_XX_XX_XX_XX"
  //             std::replace(devAddr.begin(), devAddr.end(), '_', ':'); // → XX:XX:XX:XX:XX:XX
//
  //             auto erased = devices_.erase(devAddr);
  //             if (erased > 0) {
  //                 spdlog::info("[BT] Removed device: {}", devAddr);
  //             }
  //         }
//
  //         publish_scan_result(build_device_list_json());
  //     });

    om_proxy_->finishRegistration();

    // 4) Discovery 시작
    auto adapter = sdbus::createProxy(*bus_, "org.bluez", "/org/bluez/hci0");
    adapter->callMethod("StartDiscovery").onInterface("org.bluez.Adapter1");
    spdlog::info("[BT] Discovery started...");

    // 5) 이벤트 루프 → 시그널 기다리기
    bus_->enterEventLoopAsync();
}

void BluetoothService::connect(const std::string& mac) {
    spdlog::info("[BT] Connecting to {}", mac);

    // 1. "AA:BB:CC:DD:EE:FF" → "dev_AA_BB_CC_DD_EE_FF"
    std::string path = "/org/bluez/hci0/dev_" + mac;
    std::replace(path.begin(), path.end(), ':', '_');

    try {
        // 2. 디바이스 프록시 생성
        auto devProxy = sdbus::createProxy(*bus_, "org.bluez", path.c_str());

        // 3. Connect() 메서드 호출
        devProxy->callMethod("Connect").onInterface("org.bluez.Device1");
        spdlog::info("[BT] Connect() called on {}", path);

    } catch (const sdbus::Error& e) {
        spdlog::error("[BT] Failed to connect {} : {}", mac, e.what());
    }
}

std::string BluetoothService::build_device_list_json() {
    nlohmann::json j = nlohmann::json::array();
    for (auto& [addr, name] : devices_) {
        j.push_back({{"addr", addr}, {"name", name}});
    }

    return j.dump();
}

void BluetoothService::publish_scan_result(const std::string& json_list) {
    zmq::message_t msg(json_list.begin(), json_list.end());
    try {
        pub_socket_.send(msg, zmq::send_flags::none);
        spdlog::info("[BT] Published scan result: {}", json_list);
    } catch (const zmq::error_t& e) {
        spdlog::error("[BT] Failed to publish scan result: {}", e.what());
    }
}
