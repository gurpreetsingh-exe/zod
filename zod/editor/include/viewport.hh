#pragma once

#include "backend.hh"
#include "camera.hh"
#include "widgets/panel.hh"

namespace zod {

class Viewport : public Panel {
public:
  Viewport();
  auto on_event(Event& event) -> void override;
  auto draw() -> void override;
  auto calculate(f32 x, f32 y, f32 w, f32 h) -> void override {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }

private:
  Shared<GPUShader> m_shader;
  Shared<GPUFrameBuffer> m_framebuffer;
  Shared<GPUBatch> m_batch;
  Camera m_camera;
};

} // namespace zod
