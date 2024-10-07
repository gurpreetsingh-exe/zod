#pragma once

#include "backend.hh"
#include "camera.hh"
#include "font.hh"
#include "nodes/node_tree.hh"

namespace zod {

class NodeEditor {
public:
  NodeEditor();
  auto update() -> void;
  auto draw_props() -> void;
  auto get_node_tree() -> Shared<NodeTree> { return m_node_tree; }

private:
  auto add_node(usize, vec2) -> void;

  template <typename UpdateFn>
  auto update_node(UpdateFn fn) -> void {
    ZASSERT(m_active > 0);
    auto* node = m_node_tree->node_from_id(m_active);
    fn(node);
    m_node_ssbo->update_data(node->type, sizeof(NodeType),
                             (m_active - 1) * sizeof(NodeType));
  }

private:
  f32 m_width;
  f32 m_height;
  Shared<GPUShader> m_shader;
  Shared<GPUUniformBuffer> m_camera_ubo;
  Shared<GPUStorageBuffer> m_node_ssbo;
  Shared<GPUShader> m_node_shader;
  Shared<GPUFrameBuffer> m_framebuffer;
  Shared<GPUBatch> m_batch;
  Shared<GPUShader> m_line_shader;
  Shared<GPUBatch> m_curves;
  Shared<NodeTree> m_node_tree;
  Shared<ICamera> m_camera;
  bool m_node_add = false;
  u32 m_active = 0;
  Unique<Font> m_font;
  vec2 m_last_mouse_pos = vec2(0);
};

} // namespace zod
