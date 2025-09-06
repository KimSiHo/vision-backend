#pragma once

#include <gst/gst.h>

#include <string>

class PipelineWrapper {
public:
  virtual ~PipelineWrapper() {}
  virtual void setUri(const std::string& uri) = 0;
  virtual void play() = 0;
  virtual void pause() = 0;
  virtual void stop() = 0;
  virtual GstElement* getRawPipeline() const = 0;
};
