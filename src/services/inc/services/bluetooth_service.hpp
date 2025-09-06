#pragma once

#include <string>
#include <map>

#include <sdbus-c++/sdbus-c++.h>
#include <zmq.hpp>

class BluetoothService {
public:
    BluetoothService();
    ~BluetoothService() = default;

    void power_on();
    void power_off();
    void scan();
    void connect(const std::string& device);

private:
    std::string build_device_list_json();
    void publish_scan_result(const std::string& json_list);

    std::unique_ptr<sdbus::IConnection> bus_;
    std::unique_ptr<sdbus::IProxy> om_proxy_;

    zmq::socket_t pub_socket_;
    std::map<std::string, std::string> devices_;
};
