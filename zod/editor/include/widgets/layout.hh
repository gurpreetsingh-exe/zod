#pragma once

#include "widgets/panel.hh"

namespace zod {

class Layout {
public:
  Layout() : m_areas(std::vector<Unique<Panel>>()) {}

public:
  auto add_area(Unique<Panel>) -> void;
  auto on_event(Event&) -> void;
  auto active() -> Panel*;
  auto draw(Geometry&) -> void;

private:
  std::vector<Unique<Panel>> m_areas;
};

} // namespace zod
