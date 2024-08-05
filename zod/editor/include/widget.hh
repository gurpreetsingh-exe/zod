#pragma once

#include "base/math.hh"

namespace zod {

inline usize widget_id = 0;

class Widget {
public:
  f32 x, y, w, h;
  usize id;

public:
  Widget() : id(widget_id++) {}
  virtual ~Widget() = default;
  virtual auto calculate(f32 /* x */, f32 /* y */, f32 /* width */,
                         f32 /* height */) -> void = 0;
  virtual auto generate(std::vector<vec2>&) -> void = 0;
  virtual auto on_event(Event& event) -> void = 0;
  virtual auto get_widget(f32, f32) -> Widget* { return this; }
};

} // namespace zod
