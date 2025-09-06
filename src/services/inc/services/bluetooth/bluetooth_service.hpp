#pragma once

#include <string>
#include <map>

#include <sdbus-c++/sdbus-c++.h>

#include "common/zmq/pub_socket.hpp"

class BluetoothService {
public:
    BluetoothService(PubSocket& pubSocket);

    void power_on();
    void power_off();
    void scan();
    void connect(const std::string& device);

private:
    std::string build_device_list_json();
    void publish_scan_result(const std::string& json_list);

    std::unique_ptr<sdbus::IConnection> bus_;
    std::unique_ptr<sdbus::IProxy> om_proxy_;
    std::map<std::string, std::string> devices_;
    PubSocket& pubSocket_;
};
