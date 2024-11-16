#pragma once

#include "widgets/panel.hh"

namespace zod {

class Layout {
public:
  Layout() : m_areas(std::vector<Shared<SPanel>>()) {}

public:
  auto add_area(Shared<SPanel>) -> void;
  auto on_event(Event&) -> void;
  auto active() -> SPanel*;
  auto draw(Geometry&) -> void;

private:
  std::vector<Shared<SPanel>> m_areas;
};

} // namespace zod
