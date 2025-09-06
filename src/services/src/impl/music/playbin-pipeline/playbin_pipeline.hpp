#pragma once

#include <string>

#include <gst/gst.h>

#include "services/music/pipeline_wrapper.hpp"

class PlaybinPipeline : public PipelineWrapper {
public:
    PlaybinPipeline();
    ~PlaybinPipeline() override;

    void set_uri(const std::string& path) override;
    void play() override;
    void pause() override;
    void stop() override;

    GstElement* get_raw_pipeline() const override { return pipeline; }

private:
    GstElement* pipeline = nullptr;
};
