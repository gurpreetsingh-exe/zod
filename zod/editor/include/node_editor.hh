#pragma once

#include "application/context.hh"
#include "backend.hh"
#include "font.hh"
#include "nodes/node_tree.hh"
#include "widgets/panel.hh"

namespace zod {

class NodeEditor : public Panel {
public:
  NodeEditor();
  auto update() -> void;

private:
  auto add_node(usize) -> void;
  auto draw_imp(Geometry&) -> void override { update(); }
  auto on_event_imp(Event&) -> void override;

  template <typename UpdateFn>
  auto update_node(UpdateFn fn) -> void {
    auto* node = ZCtxt::get().get_node_tree()->get_active();
    if (not node) {
      return;
    }
    fn(node);
    m_node_ssbo->update_data(node->type, sizeof(NodeType),
                             (node->type->id - 1) * sizeof(NodeType));
  }

private:
  f32 m_width;
  f32 m_height;
  Shared<GPUShader> m_shader;
  Shared<GPUStorageBuffer> m_node_ssbo;
  Shared<GPUShader> m_node_shader;
  Shared<GPUBatch> m_batch;
  Shared<GPUShader> m_line_shader;
  Shared<GPUBatch> m_curves;
  bool m_node_add = false;
  Unique<Font> m_font;
};

} // namespace zod
