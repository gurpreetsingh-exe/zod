#pragma once

#include "backend.hh"
#include "widgets/panel.hh"

namespace zod {

class Outliner : public Panel {
public:
  Outliner();
  auto on_event(Event& event) -> void override;
  auto draw(DrawData&) -> void override;
  auto calculate(f32 x, f32 y, f32 w, f32 h) -> void override {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }

private:
  Shared<GPUShader> m_shader;
  Shared<GPUBatch> m_batch;
};

} // namespace zod
