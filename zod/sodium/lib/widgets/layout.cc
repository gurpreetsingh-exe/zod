#include "widgets/layout.hh"
#include "gpu/timer.hh"

namespace zod {

auto Layout::add_area(SharedPtr<SPanel> panel) -> void {
  m_name_idx_mapping[panel->name] = m_areas.size();
  m_areas.push_back(panel);
}

auto Layout::on_event(Event& event) -> void {
  for (const auto& node : m_areas) { node->on_event(event); }
}

auto Layout::active() const -> SPanel* {
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

auto Layout::area(const String& name) const -> SharedPtr<SPanel> {
  const auto idx = m_name_idx_mapping.at(name);
  return m_areas[idx];
}

} // namespace zod
