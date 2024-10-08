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
  Panel(std::string name, Unique<ICamera> camera)
      : Widget(std::move(name)), m_camera(std::move(camera)),
        m_uniform_buffer(GPUBackend::get().create_uniform_buffer(
            sizeof(CameraUniformBufferStorage))),
        m_framebuffer(GPUBackend::get().create_framebuffer(64.0f, 64.0f)) {}
  auto on_event(Event&) -> void override {}

protected:
  Unique<ICamera> m_camera;
  Shared<GPUUniformBuffer> m_uniform_buffer;
  Shared<GPUFrameBuffer> m_framebuffer;
};

} // namespace zod
