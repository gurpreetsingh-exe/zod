#include "widgets/layout.hh"
#include "gpu/timer.hh"

namespace zod {

auto Layout::add_area(SharedPtr<SPanel> panel) -> void {
  m_areas.push_back(std::move(panel));
}

auto Layout::on_event(Event& event) -> void {
  for (const auto& node : m_areas) { node->on_event(event); }
}

auto Layout::active() -> SPanel* {
  for (const auto& area : m_areas) {
    if (area->get_active()) {
      return area.get();
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
