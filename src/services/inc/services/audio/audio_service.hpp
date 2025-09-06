#pragma once

#include <pulse/pulseaudio.h>

#include <optional>
#include <string>

class AudioService {
public:
  AudioService();
  ~AudioService();

  void setSink(const std::string& sink_name);
  const std::string& getSink() const;
  void setVolume(int percent);
  std::optional<int> getVolume() const;

private:
  static void serverInfoCb(pa_context* context, const pa_server_info* info, void* user_data);
  static void sinkInfoCb(pa_context* context, const pa_sink_info* info, int eol, void* user_data);

  std::string sink_;
  pa_mainloop* mainloop_;
  pa_mainloop_api* mainloop_api_;
  pa_context* context_;
};
