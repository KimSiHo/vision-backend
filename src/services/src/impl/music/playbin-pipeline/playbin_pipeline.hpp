#pragma once

#include <gst/gst.h>

#include <string>

#include "services/music/pipeline_wrapper.hpp"

class PlaybinPipeline : public PipelineWrapper {
public:
  PlaybinPipeline();
  ~PlaybinPipeline() override;

  void setUri(const std::string& path) override;
  void play() override;
  void pause() override;
  void stop() override;

  GstElement* getRawPipeline() const override { return pipeline_; }

private:
  GstElement* pipeline_ = nullptr;
};
