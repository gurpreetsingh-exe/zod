#pragma once

#include "gpu/pipeline.hh"

namespace zod {

class GLPipeline : public GPUPipeline {
public:
  GLPipeline(PipelineSpec spec) : GPUPipeline(spec) {}
};

} // namespace zod
