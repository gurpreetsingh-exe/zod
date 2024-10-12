#include "widgets/layout.hh"
#include "application/context.hh"

namespace zod {

auto Layout::add_area(Unique<Panel> panel) -> void {
  m_areas.push_back(std::move(panel));
}

auto Layout::on_event(Event& event) -> void {
  for (const auto& node : m_areas) { node->on_event(event); }
}

auto Layout::active() -> Panel* {
  for (const auto& node : m_areas) {
    if (node->get_active()) {
      return node.get();
    }
  }

  return nullptr;
}

auto Layout::draw(Geometry& g) -> void {
  for (const auto& node : m_areas) {
    GPU_TIME(node->name, { node->draw(g); });
  }
}

} // namespace zod
