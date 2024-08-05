#pragma once

#include "widgets/panel.hh"
#include "backend.hh"

namespace zod {

class Viewport : public Panel {
public:
  Viewport();
  auto on_event(Event& event) -> void override;
  auto draw() -> void override;

private:
  Shared<GPUShader> m_shader;
  Shared<GPUFrameBuffer> m_framebuffer;
};

} // namespace zod
