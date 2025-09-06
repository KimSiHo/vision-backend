#pragma once
#include <string>
#include <optional>

class AudioService {
public:
    AudioService();
    ~AudioService();

    void set_sink(const std::string& sink_name);
    void set_volume(int percent);
    std::optional<int> get_volume() const;

private:
    std::string sink_;
};
