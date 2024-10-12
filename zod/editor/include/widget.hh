#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "base/math.hh"
#include "event.hh"

namespace zod {

inline usize widget_id = 0;

struct Geometry {
  std::vector<vec2> points;
  std::vector<vec4> colors;

  auto clear() -> void {
    points.clear();
    colors.clear();
  }
};

class Widget {
public:
  usize id;
  std::string name;

protected:
  bool m_needs_update = true;

public:
  Widget(std::string n) : id(widget_id++), name(std::move(n)) {}
  virtual ~Widget() = default;
  virtual auto on_event(Event& event) -> void = 0;

protected:
  virtual auto draw_imp(Geometry&) -> void = 0;
};

} // namespace zod
