#pragma once

#include "../pipeline.hh"

namespace zod {

class GLPipeline : public GPUPipeline {
public:
  GLPipeline(PipelineSpec spec) : GPUPipeline(spec) {}
};

} // namespace zod
