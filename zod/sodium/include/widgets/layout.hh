#pragma once

#include "widgets/panel.hh"

namespace zod {

class Layout {
public:
  Layout() : m_areas(Vector<SharedPtr<SPanel>>()) {}

public:
  auto add_area(SharedPtr<SPanel>) -> void;
  auto on_event(Event&) -> void;
  auto active() const -> SPanel*;
  auto draw(Geometry&) -> void;
  auto area(const String&) const -> SharedPtr<SPanel>;

private:
  Vector<SharedPtr<SPanel>> m_areas;
  std::unordered_map<String, usize> m_name_idx_mapping = {};
};

} // namespace zod
