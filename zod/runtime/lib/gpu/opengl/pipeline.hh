#pragma once

#include "gpu/pipeline.hh"

namespace zod {

class GLPipeline : public GPUPipeline {
public:
  GLPipeline(GPUPipelineCreateInfo info) : GPUPipeline(info) {}
};

} // namespace zod
