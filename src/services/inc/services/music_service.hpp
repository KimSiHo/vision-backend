#pragma once

#include <string>

#include "services/pipeline_wrapper.hpp"

class MusicService {
public:
    enum class PipelineMode {
        Custom,
        Playbin
    };

    explicit MusicService(PipelineMode mode);
    ~MusicService();

    void play();
    void stop();
    void pause();
    void next();
    void prev();

private:
    PipelineWrapper* pipeline;
};
