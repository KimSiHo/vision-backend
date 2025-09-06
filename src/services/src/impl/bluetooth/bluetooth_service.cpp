#include "services/bluetooth/bluetooth_service.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include "common/utils/json.hpp"
#include "common/utils/logging.hpp"
#include "config/zmq_config.hpp"

BluetoothService::BluetoothService(PubSocket& pubSocket)
    : pubSocket_(pubSocket) { }

void BluetoothService::power_on() {
    auto bus = sdbus::createSystemBusConnection();

    const char* adapterPath = "/org/bluez/hci0";
    auto adapter = sdbus::createProxy(*bus, "org.bluez", adapterPath);

    adapter->callMethod("Set")
        .onInterface("org.freedesktop.DBus.Properties")
        .withArguments("org.bluez.Adapter1", "Powered", sdbus::Variant(true));

    SPDLOG_SERVICE_INFO("Bluetooth Power ON Success");
}

void BluetoothService::power_off() {
    auto bus = sdbus::createSystemBusConnection();

    const char* adapterPath = "/org/bluez/hci0";
    auto adapter = sdbus::createProxy(*bus, "org.bluez", adapterPath);

    adapter->callMethod("Set")
        .onInterface("org.freedesktop.DBus.Properties")
        .withArguments("org.bluez.Adapter1", "Powered", sdbus::Variant(false));

    SPDLOG_SERVICE_INFO("Bluetooth Power OFF Success");
}

void BluetoothService::scan() {
    SPDLOG_SERVICE_INFO("Scanning...");

    // 1) DBus 연결
    bus_ = sdbus::createSystemBusConnection();
    om_proxy_ = sdbus::createProxy(*bus_, "org.bluez", "/");

    // 2) InterfacesAdded 시그널 핸들러 등록
    om_proxy_->uponSignal("InterfacesAdded")
        .onInterface("org.freedesktop.DBus.ObjectManager")
        .call([this](const sdbus::ObjectPath& path,
                     const std::map<std::string, std::map<std::string, sdbus::Variant>>& ifaces) {
            SPDLOG_SERVICE_INFO("InterfacesAdded for path: {}", path.c_str());

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
                    SPDLOG_SERVICE_INFO("Added device: {} ({})", addr, name);
                    publish_scan_result(build_device_list_json());
                }
            }
        });

    // 3) InterfacesRemoved 시그널 핸들러 등록
    om_proxy_->uponSignal("InterfacesRemoved")
        .onInterface("org.freedesktop.DBus.ObjectManager")
        .call([this](const sdbus::ObjectPath& path,
                     const std::vector<std::string>& ifaces) {
            SPDLOG_SERVICE_INFO("InterfacesRemoved: {}", path.c_str());

            std::string devPath = path;
            auto pos = devPath.find("dev_");
            if (pos != std::string::npos) {
                std::string devAddr = devPath.substr(pos + 4); // "XX_XX_XX_XX_XX_XX"
                std::replace(devAddr.begin(), devAddr.end(), '_', ':'); // → XX:XX:XX:XX:XX:XX

                auto erased = devices_.erase(devAddr);
                if (erased > 0) {
                    SPDLOG_SERVICE_INFO("Removed device: {}", devAddr);
                }
            }

            publish_scan_result(build_device_list_json());
        });

    om_proxy_->finishRegistration();

    // 4) Discovery 시작
    auto adapter = sdbus::createProxy(*bus_, "org.bluez", "/org/bluez/hci0");
    adapter->callMethod("StartDiscovery").onInterface("org.bluez.Adapter1");

    // 5) 이벤트 루프 → 시그널 기다리기
    bus_->enterEventLoopAsync();
}

void BluetoothService::connect(const std::string& mac) {
    SPDLOG_SERVICE_INFO("Connecting to {}", mac);

    // 1. "AA:BB:CC:DD:EE:FF" → "dev_AA_BB_CC_DD_EE_FF"
    std::string path = "/org/bluez/hci0/dev_" + mac;
    std::replace(path.begin(), path.end(), ':', '_');

    try {
        // 2. 디바이스 프록시 생성
        auto devProxy = sdbus::createProxy(*bus_, "org.bluez", path.c_str());

        // 3. Connect() 메서드 호출
        devProxy->callMethod("Connect").onInterface("org.bluez.Device1");
        SPDLOG_SERVICE_INFO("Connect() called on {}", path);
    } catch (const sdbus::Error& e) {
        SPDLOG_SERVICE_ERROR("Failed to connect {} : {}", mac, e.what());
    }
}

std::string BluetoothService::build_device_list_json() {
    AppCommon::json j = AppCommon::json::array();
    for (auto& [addr, name] : devices_) {
        j.push_back({{"addr", addr}, {"name", name}});
    }

    return j.dump();
}

void BluetoothService::publish_scan_result(const std::string& json_list) {
    pubSocket_.publish(std::string(AppConfig::TOPIC_BLUETOOTH), json_list);
}
