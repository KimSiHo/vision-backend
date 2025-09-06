#include "services/bluetooth/bluetooth_service.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include "common/utils/json.hpp"
#include "common/utils/logging.hpp"
#include "config/zmq_config.hpp"

BluetoothService::BluetoothService(PubSocket& pub_socket) : pub_socket_(pub_socket) {}

void BluetoothService::powerOn() {
  auto bus = sdbus::createSystemBusConnection();
  const char* adapter_path = "/org/bluez/hci0";
  auto adapter = sdbus::createProxy(*bus, "org.bluez", adapter_path);

  adapter->callMethod("Set")
      .onInterface("org.freedesktop.DBus.Properties")
      .withArguments("org.bluez.Adapter1", "Powered", sdbus::Variant(true));

  SPDLOG_SERVICE_INFO("Bluetooth Power ON Success");
}

void BluetoothService::powerOff() {
  auto bus = sdbus::createSystemBusConnection();
  const char* adapter_path = "/org/bluez/hci0";
  auto adapter = sdbus::createProxy(*bus, "org.bluez", adapter_path);

  adapter->callMethod("Set")
      .onInterface("org.freedesktop.DBus.Properties")
      .withArguments("org.bluez.Adapter1", "Powered", sdbus::Variant(false));

  SPDLOG_SERVICE_INFO("Bluetooth Power OFF Success");
}

void BluetoothService::scan() {
  SPDLOG_SERVICE_INFO("Scanning...");

  bus_ = sdbus::createSystemBusConnection();
  om_proxy_ = sdbus::createProxy(*bus_, "org.bluez", "/");

  om_proxy_->uponSignal("InterfacesAdded")
      .onInterface("org.freedesktop.DBus.ObjectManager")
      .call([this](const sdbus::ObjectPath& path,
                   const std::map<std::string, std::map<std::string, sdbus::Variant>>& ifaces) {
        SPDLOG_SERVICE_INFO("InterfacesAdded for path: {}", path.c_str());

        auto it = ifaces.find("org.bluez.Device1");
        if (it != ifaces.end()) {
          const auto& props = it->second;

          std::string addr;
          std::string name;

          if (auto p = props.find("Address"); p != props.end()) {
            addr = p->second.get<std::string>();
          }
          if (auto p = props.find("Name"); p != props.end()) {
            name = p->second.get<std::string>();
          }

          if (!addr.empty()) {
            devices_[addr] = name;
            SPDLOG_SERVICE_INFO("Added device: {} ({})", addr, name);
            publishScanResult(buildDeviceListJson());
          }
        }
      });

  om_proxy_->uponSignal("InterfacesRemoved")
      .onInterface("org.freedesktop.DBus.ObjectManager")
      .call([this](const sdbus::ObjectPath& path, const std::vector<std::string>& ifaces) {
        SPDLOG_SERVICE_INFO("InterfacesRemoved: {}", path.c_str());

        std::string dev_path = path;
        auto pos = dev_path.find("dev_");
        if (pos != std::string::npos) {
          std::string dev_addr = dev_path.substr(pos + 4);
          std::replace(dev_addr.begin(), dev_addr.end(), '_', ':');

          auto erased = devices_.erase(dev_addr);
          if (erased > 0) {
            SPDLOG_SERVICE_INFO("Removed device: {}", dev_addr);
          }
        }

        publishScanResult(buildDeviceListJson());
      });

  om_proxy_->finishRegistration();

  auto adapter = sdbus::createProxy(*bus_, "org.bluez", "/org/bluez/hci0");
  adapter->callMethod("StartDiscovery").onInterface("org.bluez.Adapter1");

  bus_->enterEventLoopAsync();
}

void BluetoothService::connect(const std::string& mac) {
  SPDLOG_SERVICE_INFO("Connecting to {}", mac);

  std::string path = "/org/bluez/hci0/dev_" + mac;
  std::replace(path.begin(), path.end(), ':', '_');

  try {
    auto dev_proxy = sdbus::createProxy(*bus_, "org.bluez", path.c_str());
    dev_proxy->callMethod("Connect").onInterface("org.bluez.Device1");
    SPDLOG_SERVICE_INFO("Connect() called on {}", path);
  } catch (const sdbus::Error& e) {
    SPDLOG_SERVICE_ERROR("Failed to connect {} : {}", mac, e.what());
  }
}

std::string BluetoothService::buildDeviceListJson() {
  app_common::Json json = app_common::Json::array();
  for (auto& [addr, name] : devices_) {
    json.push_back({{"addr", addr}, {"name", name}});
  }
  return json.dump();
}

void BluetoothService::publishScanResult(const std::string& json_list) {
  pub_socket_.publish(std::string(app_config::kTopicBluetooth), json_list);
}
