#pragma once

#include "backend.hh"
#include "camera.hh"
#include "input.hh"
#include "widget.hh"

namespace zod {

struct CameraUniformBufferStorage {
  mat4 view_projection;
  vec4 direction;
};

class IOperator;

inline Shared<GPUUniformBuffer> unit_uniform_buffer = nullptr;

class SPanel : public SWidget {
public:
  SPanel(std::string /* name */, Unique<ICamera>, bool /* padding */ = true);
  auto on_event(Event&) -> void override;
  auto compute_desired_size() -> void override;
  virtual auto on_event_imp(Event&) -> void {};
  auto draw(Geometry&) -> void;
  auto get_active() -> bool { return m_active; }
  auto camera() const -> const ICamera& { return *m_camera; }
  auto relative_mouse_position() const -> vec2 {
    auto position = Input::get_mouse_pos() - m_position + vec2(0, 43);
    position.y = m_size.y - position.y;
    return position;
  }
  auto region_space_mouse_position() const -> vec2 {
    return vec2(m_camera->screen_to_world(relative_mouse_position()));
  }
  auto bind_window_space_uniform_buffer() -> void;

protected:
  bool m_padding = true;
  bool m_active = false;
  Unique<ICamera> m_camera;
  Shared<GPUUniformBuffer> m_uniform_buffer;
  Shared<GPUFrameBuffer> m_framebuffer;
  std::unordered_map<KeyCode, std::function<void()>> m_keymaps = {};
  IOperator* m_active_operator = nullptr;
  std::string m_debug_message = std::string();
};

} // namespace zod
