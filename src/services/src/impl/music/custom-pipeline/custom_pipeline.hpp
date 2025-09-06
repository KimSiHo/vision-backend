#pragma once

#include <gst/gst.h>

#include <string>

#include "services/music/pipeline_wrapper.hpp"

class CustomPipeline : public PipelineWrapper {
public:
  CustomPipeline();
  ~CustomPipeline() override;

  void setUri(const std::string& uri) override;
  void play() override;
  void pause() override;
  void stop() override;

  GstElement* getRawPipeline() const override { return pipeline_; }

private:
  GstElement* pipeline_ = nullptr;
  GstElement* source_bin_ = nullptr;
  GstElement* sink_bin_ = nullptr;
};
