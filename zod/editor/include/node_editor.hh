#pragma once

#include "backend.hh"
#include "camera.hh"

namespace zod {

class NodeEditor {
public:
  NodeEditor();
  auto update() -> void;

private:
  f32 m_width;
  f32 m_height;
  Shared<GPUShader> m_shader;
  Shared<GPUFrameBuffer> m_framebuffer;
  Shared<GPUBatch> m_batch;
  Camera m_camera;
};

} // namespace zod
