#pragma once

#include "backend.hh"
#include "camera.hh"
#include "font.hh"
#include "node_tree.hh"

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
  Shared<NodeTree> m_node_tree;
  OrthographicCamera m_camera;
  bool m_node_add = false;
  u32 m_active = 0;
  Unique<Font> m_font;
};

} // namespace zod
