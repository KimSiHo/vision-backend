#pragma once

#include <string_view>

namespace app_config {
// Endpoints
inline constexpr std::string_view kControlEndpoint = "ipc:///tmp/app.control";
inline constexpr std::string_view kEventEndpoint = "ipc:///tmp/app.events";

// Topics
inline constexpr std::string_view kTopicDetections = "det";
inline constexpr std::string_view kTopicBluetooth = "blt";
inline constexpr std::string_view kTopicTrackChanged = "TRACK_CHANGED";
}  // namespace app_config
