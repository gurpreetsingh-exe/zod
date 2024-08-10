#pragma once

#include "backend.hh"
#include "camera.hh"

namespace zod {

class Viewport {
public:
  Viewport();
  auto update(Shared<GPUBatch>) -> void;

private:
  f32 m_width;
  f32 m_height;
  Shared<GPUShader> m_shader;
  Shared<GPUFrameBuffer> m_framebuffer;
  Camera m_camera;
};

} // namespace zod
