#pragma once

#include <string>

#include <gst/gst.h>

#include "services/pipeline_wrapper.hpp"

class CustomPipeline : public PipelineWrapper {
public:
    CustomPipeline();
    ~CustomPipeline() override;

    void set_uri(const std::string& uri) override;
    void play() override;
    void pause() override;
    void stop() override;

private:
    GstElement* pipeline = nullptr;
    GstElement* sourceBin = nullptr;
    GstElement* audioBin = nullptr;
};
