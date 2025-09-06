#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <string>

#include "common/zmq/pub_socket.hpp"

class BluetoothService {
public:
  BluetoothService(PubSocket& pub_socket);

  void powerOn();
  void powerOff();
  void scan();
  void connect(const std::string& device);

private:
  std::string buildDeviceListJson();
  void publishScanResult(const std::string& json_list);

  std::unique_ptr<sdbus::IConnection> bus_;
  std::unique_ptr<sdbus::IProxy> om_proxy_;
  std::map<std::string, std::string> devices_;
  PubSocket& pub_socket_;
};
