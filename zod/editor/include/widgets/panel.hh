#pragma once

#include "camera.hh"
#include "widget.hh"

namespace zod {

struct CameraUniformBufferStorage {
  mat4 view_projection;
  vec4 direction;
};

class Panel : public Widget {
public:
  Panel(std::string /* name */, Unique<ICamera>, bool /* padding */ = true);
  auto on_event(Event&) -> void override;
  auto draw(Geometry&) -> void;
  auto get_active() -> bool { return m_active; }

protected:
  bool m_padding = true;
  bool m_active = false;
  Unique<ICamera> m_camera;
  Shared<GPUUniformBuffer> m_uniform_buffer;
  Shared<GPUFrameBuffer> m_framebuffer;
};

} // namespace zod
