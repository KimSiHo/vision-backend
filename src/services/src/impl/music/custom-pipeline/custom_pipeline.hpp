#pragma once

#include <string>

#include <gst/gst.h>

#include "services/music/pipeline_wrapper.hpp"

class CustomPipeline : public PipelineWrapper {
public:
    CustomPipeline();
    ~CustomPipeline() override;

    void set_uri(const std::string& uri) override;
    void play() override;
    void pause() override;
    void stop() override;

    GstElement* get_raw_pipeline() const override { return pipeline; }

private:
    GstElement* pipeline = nullptr;
    GstElement* sourceBin = nullptr;
    GstElement* sinkBin = nullptr;
};
