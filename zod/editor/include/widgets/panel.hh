#pragma once

#include "backend.hh"
#include "camera.hh"
#include "widget.hh"

namespace zod {

struct CameraUniformBufferStorage {
  mat4 view_projection;
  vec4 direction;
};

class Operator;

class Panel : public Widget {
public:
  Panel(std::string /* name */, Unique<ICamera>, bool /* padding */ = true);
  auto on_event(Event&) -> void override;
  virtual auto on_event_imp(Event&) -> void {};
  auto draw(Geometry&) -> void;
  auto get_active() -> bool { return m_active; }
  auto camera() const -> const ICamera& { return *m_camera; }
  auto relative_mouse_position() const -> vec2 {
    return Input::get_mouse_pos() - m_position + vec2(0, 43);
  }
  auto region_space_mouse_position() const -> vec2 {
    return vec2(m_camera->screen_to_world(relative_mouse_position()));
  }

protected:
  bool m_padding = true;
  bool m_active = false;
  vec2 m_position;
  vec2 m_size;
  Unique<ICamera> m_camera;
  Shared<GPUUniformBuffer> m_uniform_buffer;
  Shared<GPUFrameBuffer> m_framebuffer;
  std::unordered_map<Key, std::function<void()>> m_keymaps = {};
  Operator* m_active_operator = nullptr;
  std::string m_debug_message = std::string();
};

} // namespace zod
