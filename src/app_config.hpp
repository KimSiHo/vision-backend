#pragma once

#include <zmq.hpp>

class AppConfig {
public:
    static zmq::context_t& context() {
        static zmq::context_t ctx{1};
        return ctx;
    }

private:
    AppConfig() = default;
};
