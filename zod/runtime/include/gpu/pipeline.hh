#pragma once

#include "shader.hh"

namespace zod {

enum class GPUPrimitive {
  Points,
  Lines,
  LineStrip,
  Triangles,
  TriangleStrip,
  TriangleFan,
};

enum class GPUPrimitiveDrawMode {
  Fill,
  Line,
  Point,
};

enum class GPUCullMode {
  None,
  Front,
  Back,
  FrontAndBack,
};

struct PipelineSpec {
  SharedPtr<GPUShader> shader;
  GPUPrimitive primitive = GPUPrimitive::Triangles;
  GPUPrimitiveDrawMode draw_mode = GPUPrimitiveDrawMode::Fill;
  GPUCullMode cull_mode = GPUCullMode::None;
};

class GPUPipeline {
protected:
  GPUPipeline(PipelineSpec spec) : m_shader(spec.shader) {}

public:
  virtual ~GPUPipeline() = default;

protected:
  SharedPtr<GPUShader> m_shader;
};

} // namespace zod
