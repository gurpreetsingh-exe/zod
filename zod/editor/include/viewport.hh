#pragma once

#include "widgets/panel.hh"
#include "backend.hh"
#include "camera.hh"

namespace zod {

class Viewport : public Panel {
public:
  Viewport();
  auto on_event(Event& event) -> void override;
  auto draw() -> void override;

private:
  Shared<GPUShader> m_shader;
  Shared<GPUFrameBuffer> m_framebuffer;
  Shared<GPUBatch> m_batch;
  Camera m_camera;
};

} // namespace zod
