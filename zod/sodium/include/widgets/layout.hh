#pragma once

#include "widgets/panel.hh"

namespace zod {

class Layout {
public:
  Layout() : m_areas(Vector<SharedPtr<SPanel>>()) {}

public:
  auto add_area(SharedPtr<SPanel>) -> void;
  auto on_event(Event&) -> void;
  auto active() -> SPanel*;
  auto draw(Geometry&) -> void;

private:
  Vector<SharedPtr<SPanel>> m_areas;
};

} // namespace zod
