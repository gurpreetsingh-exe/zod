#pragma once

#include "sodium/gui.hh"

namespace zod {

class Layout {
public:
  Layout() : m_areas(Vector<SharedPtr<sodium::area>>()) {}

public:
  auto add_area(SharedPtr<sodium::area>) -> void;
  auto on_event(Event&) -> void;
  auto active() const -> sodium::area*;
  auto draw() -> void;
  auto area(const String&) const -> SharedPtr<sodium::area>;

private:
  Vector<SharedPtr<sodium::area>> m_areas;
  std::unordered_map<String, usize> m_name_idx_mapping = {};
};

} // namespace zod
