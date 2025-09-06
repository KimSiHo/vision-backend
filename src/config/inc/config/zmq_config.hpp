#pragma once

#include <string_view>

namespace AppConfig {
    // Endpoints
    inline constexpr std::string_view CONTROL_ENDPOINT = "ipc:///tmp/app.control";
    inline constexpr std::string_view EVENT_ENDPOINT   = "ipc:///tmp/app.events";

    // Topics
    inline constexpr std::string_view TOPIC_DETECTIONS = "det";
    inline constexpr std::string_view TOPIC_BLUETOOTH = "blt";
    inline constexpr std::string_view TOPIC_TRACK_CHANGED = "TRACK_CHANGED";
}
