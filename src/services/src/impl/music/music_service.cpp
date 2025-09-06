#include "services/music_service.hpp"

#include <vector>
#include <string>

#include <spdlog/spdlog.h>

#include "impl/music/playbin-pipeline/playbin_pipeline.hpp"
#include "impl/music/custom-pipeline/custom_pipeline.hpp"

namespace {
    std::vector<std::string> playlist = {
        "/opt/assets/track1.mp3",
        "/opt/assets/track2.mp3",
        "/opt/assets/track3.mp3"
    };

    int current_index = 0;
}

MusicService::MusicService(PipelineMode mode) : pipeline(nullptr) {
    spdlog::info("Using {} pipeline", (mode == PipelineMode::Playbin) ? "playbin" : "custom");

    if (mode == PipelineMode::Playbin) {
        pipeline = new PlaybinPipeline();
    } else {
        pipeline = new CustomPipeline();
    }

    pipeline->set_uri(playlist[current_index]);
}


MusicService::~MusicService() {
    delete pipeline;
}

void MusicService::play() { pipeline->play(); }
void MusicService::pause() { pipeline->pause(); }
void MusicService::stop() { pipeline->stop(); }

void MusicService::next() {
    current_index = (current_index + 1) % playlist.size();
    pipeline->set_uri(playlist[current_index]);
    play();
}

void MusicService::prev() {
    current_index = (current_index - 1 + playlist.size()) % playlist.size();
    pipeline->set_uri(playlist[current_index]);
    play();
}
