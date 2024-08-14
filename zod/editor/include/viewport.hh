#pragma once

#include "backend.hh"
#include "camera.hh"

namespace zod {

class Viewport {
public:
  Viewport();
  auto update(Shared<GPUBatch>) -> void;

private:
  auto draw_cubemap() -> void;
  auto draw_grid() -> void;
  auto draw_axes() -> void;

private:
  f32 m_width;
  f32 m_height;
  Shared<GPUShader> m_shader;
  Shared<GPUBatch> m_cubemap_batch;
  Shared<GPUShader> m_cubemap_shader;
  Shared<GPUShader> m_grid_shader;
  Shared<GPUFrameBuffer> m_framebuffer;
  Shared<GPUUniformBuffer> m_camera_ubo;
  Camera m_camera;
};

} // namespace zod
