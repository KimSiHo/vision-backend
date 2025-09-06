#pragma once

#include <string>

#include <gst/gst.h>

#include "services/pipeline_wrapper.hpp"

class PlaybinPipeline : public PipelineWrapper {
public:
    PlaybinPipeline();
    ~PlaybinPipeline() override;

    void set_uri(const std::string& path) override;
    void play() override;
    void pause() override;
    void stop() override;

private:
    GstElement* pipeline = nullptr;
};
