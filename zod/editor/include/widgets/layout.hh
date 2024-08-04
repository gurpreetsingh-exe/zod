#pragma once

#include "widget.hh"

namespace zod {

class Layout {
public:
  Layout() = default;

public:
  auto add_area(Unique<Widget> widget) -> void {
    m_areas.push_back(std::move(widget));
  }

  auto calculate(f32 x, f32 y, f32 w, f32 h) -> void {
    for (const auto& node : m_areas) { node->calculate(x, y, w, h); }
  }

  // auto generate(GMesh& mesh) -> void {
  //   for (const auto& node : m_areas) { node->generate(mesh); }
  // }

private:
  std::vector<Unique<Widget>> m_areas;
};

} // namespace zod
