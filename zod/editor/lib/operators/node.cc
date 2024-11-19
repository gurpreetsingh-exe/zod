#include "operators/node.hh"
#include "context.hh"
#include "widgets/panel.hh"

namespace zod {

auto OpNodeTransform::execute(Event& event) -> u8 {
  auto* node = ZCtxt::get().get_node_tree()->get_active();
  if (not node) {
    return OP_CANCELLED;
  }
  const auto& camera = m_panel->camera();
  auto delta = vec2(camera.screen_to_world(event.mouse) -
                    camera.screen_to_world(g_last_mouse_pos));
  delta.y = -delta.y;
  node->type->location += delta;
  return OP_RUNNING;
}

///////////////////////////////////////////////////////////////

auto OpNodeLinkCreateNew::execute(Event& event) -> u8 {
  auto node_tree = ZCtxt::get().get_node_tree();
  if (event.is_key_down(Key::Escape) or
      event.is_mouse_down(MouseButton::Right)) {
    node_tree->remove_link(m_link);
    return OP_CANCELLED;
  }

  if (event.kind == Event::MouseMove or event.kind != Event::MouseDown) {
    return OP_RUNNING;
  }

  if (not m_data) {
    m_link = node_tree->add_link_partial(m_from);
    return OP_RUNNING;
  }

  ZASSERT(m_link);
  node_tree->connect_link(m_link, (Node*)m_data);
  return OP_FINISHED;
}

} // namespace zod
