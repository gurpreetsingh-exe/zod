#pragma once

#include "backend.hh"
#include "camera.hh"

namespace zod {

class NodeEditor {
public:
  NodeEditor();
  auto update() -> void;

private:
  auto add_node() -> void;

private:
  f32 m_width;
  f32 m_height;
  Shared<GPUShader> m_shader;
  Shared<GPUUniformBuffer> m_camera_ubo;
  Shared<GPUStorageBuffer> m_node_locations;
  Shared<GPUShader> m_node_shader;
  Shared<GPUFrameBuffer> m_framebuffer;
  Shared<GPUBatch> m_batch;
  Camera m_camera;
  bool m_node_add = false;
};

} // namespace zod
