#pragma once

#include <string>
#include <optional>

#include <pulse/pulseaudio.h>

class AudioService {
public:
    AudioService();
    ~AudioService();

    void setSink(const std::string& sink_name);
    const std::string& getSink() const;
    void setVolume(int percent);
    std::optional<int> getVolume() const;

private:
    // callback functions
    static void serverInfoCb(pa_context*, const pa_server_info* i, void* userdata);
    static void sinkInfoCb(pa_context*, const pa_sink_info* i, int eol, void* userdata);

    std::string sink_;
    pa_mainloop* ml_;
    pa_mainloop_api* api_;
    pa_context* ctx_;
};
