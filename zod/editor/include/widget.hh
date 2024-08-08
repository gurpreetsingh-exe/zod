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
  Shared<GPUFrameBuffer> m_framebuffer;
  bool m_needs_update = true;

public:
  Widget() : id(widget_id++) {}
  virtual ~Widget() = default;
  virtual auto calculate(f32 /* x */, f32 /* y */, f32 /* width */,
                         f32 /* height */) -> void = 0;
  virtual auto generate(std::vector<vec2>&) -> void = 0;
  virtual auto draw(DrawData& data) -> void {
    draw_background(data);
    m_needs_update = false;
  }
  virtual auto on_event(Event& event) -> void = 0;
  virtual auto get_widget(f32, f32) -> Widget* { return this; }

protected:
  template <typename T>
  auto _draw(DrawData& data, T cb) {
    draw_background(data);
    cb();
  }

  auto draw_background(DrawData& data) -> void {
    auto shader = data.shader;
    auto batch = data.batch;
    shader->bind();
    shader->uniform("u_x", x);
    shader->uniform("u_y", y);
    shader->uniform("u_width", w);
    shader->uniform("u_height", h);
    shader->uniform("u_border", border);
    shader->uniform("u_padding", padding);
    shader->uniform("u_color", glm::vec3(0.15f, 0.16f, 0.17f));
    batch->draw(shader);
  }
};

} // namespace zod
