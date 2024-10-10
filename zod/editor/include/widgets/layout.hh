#pragma once

#include "application/context.hh"
#include "widget.hh"

namespace zod {

class Layout {
public:
  Layout() : m_areas(std::vector<Unique<Panel>>()) {}

public:
  auto add_area(Unique<Panel> panel) -> void {
    m_areas.push_back(std::move(panel));
  }

  auto on_event(Event& event) -> void {
    for (const auto& node : m_areas) { node->on_event(event); }
  }

  auto draw(Geometry& g) -> void {
    for (const auto& node : m_areas) {
      GPU_TIME(node->name, { node->draw(g); });
    }
  }

private:
  std::vector<Unique<Panel>> m_areas;
};

} // namespace zod
