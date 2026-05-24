#pragma once

#include "sodium/widgets/widget.hh"

namespace zod::sodium {

struct HitTestEntry {
  WidgetPath path = {};
  Rect bounds = {};
  i32 layer = 0;
  i32 priority = 0;
  i32 order = 0;
};

class HitTestList {
public:
  auto clear() -> void;
  auto build_from(Widget&) -> void;
  auto path_at(vec2) const -> WidgetPath;

private:
  auto collect(Widget&, WidgetPath, i32 layer) -> void;

private:
  Vector<HitTestEntry> m_entries = {};
  i32 m_next_order = 0;
};

} // namespace zod::sodium
