#pragma once

#include <string>

#include <gst/gst.h>

class PipelineWrapper {
public:
    virtual ~PipelineWrapper() {}
    virtual void set_uri(const std::string& uri) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual GstElement* get_raw_pipeline() const = 0;
};
