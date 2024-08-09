#pragma once

#include "base/math.hh"
#include "event.hh"

namespace zod {

inline usize widget_id = 0;
extern int border;
extern int padding;

struct DrawData {
  Shared<GPUBatch> batch;
  Shared<GPUShader> shader;
};

class Widget {
public:
  f32 x, y, w, h;
  usize id;

protected:
  bool m_needs_update = true;

public:
  Widget() : id(widget_id++) {}
  virtual ~Widget() = default;
  virtual auto draw() -> void { m_needs_update = false; }
  virtual auto on_event(Event& event) -> void = 0;
  virtual auto get_widget(f32, f32) -> Widget* { return this; }
};

} // namespace zod
