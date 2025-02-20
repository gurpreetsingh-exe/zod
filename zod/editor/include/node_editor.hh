#pragma once

#include "context.hh"
#include "core/node_types.hh"
#include "gpu/backend.hh"
#include "widgets/panel.hh"

namespace zod {

constexpr f32 NODE_SIZE = 100.0f;

class NodeEditor : public SPanel {
public:
  NodeEditor();
  auto update() -> void;

private:
  auto add_node(usize) -> void;
  auto draw_imp(Geometry&) -> void override { update(); }
  auto on_event_imp(Event&) -> void override;

  template <typename UpdateFn>
  auto update_node(UpdateFn fn) -> void {
    auto node_tree = ZCtxt::get().get_node_tree();
    m_links.clear();
    auto& links = node_tree->get_links();
    for (const auto& link : links) {
      m_links.push_back(link.node_from->type->location + vec2(NODE_SIZE) -
                        vec2(0, 38));
      auto other_end =
          link.node_to
              ? (link.node_to->type->location + vec2(NODE_SIZE) + vec2(0, 38))
              : region_space_mouse_position();
      m_links.push_back(other_end);
      m_curves->get_buffer(0)->upload_data(m_links.data(),
                                           sizeof(vec2) * m_links.size());
    }

    auto* node = node_tree->get_active();
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
  std::vector<vec2> m_links = {};

  /// Debug
  bool m_framebuffer_bit = 0;
};

} // namespace zod
