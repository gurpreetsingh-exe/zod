#pragma once

#include "base/math.hh"
#include "event.hh"

namespace zod {

inline usize widget_id = 0;

struct DrawData {
  Shared<GPUBatch> batch;
  Shared<GPUShader> shader;
};

class Widget {
public:
  f32 x, y, w, h;
  usize id;

protected:
  Shared<GPUFrameBuffer> m_framebuffer;
  bool m_needs_update = true;

public:
  Widget() : id(widget_id++) {}
  virtual ~Widget() = default;
  virtual auto calculate(f32 /* x */, f32 /* y */, f32 /* width */,
                         f32 /* height */) -> void = 0;
  virtual auto generate(std::vector<vec2>&) -> void = 0;
  virtual auto draw(DrawData&) -> void { m_needs_update = false; }
  virtual auto on_event(Event& event) -> void = 0;
  virtual auto get_widget(f32, f32) -> Widget* { return this; }
};

} // namespace zod
