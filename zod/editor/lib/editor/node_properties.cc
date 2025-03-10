#include "node_properties.hh"
#include "editor.hh"
#include "engine/camera.hh"

namespace zod {

NodeProperties::NodeProperties()
    : SPanel("Node Properties", shared<OrthographicCamera>(64.0f, 64.0f)) {}

auto NodeProperties::update() -> void {
  auto node_tree = Editor::get().get_node_tree();
  if (not node_tree->get_active_id()) {
    return;
  }
  auto* node = node_tree->get_active();
  auto name =
      fmt::format("{}.{}", node_names[node->type->type], node->type->id);
  ImGui::SeparatorText(name.c_str());
  ImGui::Spacing();
  for (usize i = 0; i < node->props.size(); ++i) {
    auto& prop = node->props[i];
    if (draw_property(prop) and node_tree->get_visualized() == node->type->id) {
      node->update(*node);
    }
  }
}

} // namespace zod
