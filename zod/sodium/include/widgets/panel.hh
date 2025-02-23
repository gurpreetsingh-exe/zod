#pragma once

#include "application/input.hh"
#include "gpu/backend.hh"
#include "widget.hh"

namespace zod {

class ICamera;

struct CameraUniformBufferStorage {
  mat4 view_projection;
  vec4 direction;
};

class IOperator;

inline SharedPtr<GPUUniformBuffer> unit_uniform_buffer = nullptr;

class SPanel : public SWidget {
public:
  SPanel(String /* name */, UniquePtr<ICamera>, bool /* padding */ = true);
  auto on_event(Event&) -> void override;
  auto compute_desired_size() -> void override;
  virtual auto on_event_imp(Event&) -> void {};
  auto draw(Geometry&) -> void;
  auto get_active() const -> bool;
  auto camera() const -> const ICamera&;
  auto relative_mouse_position() const -> vec2;
  auto region_space_mouse_position() const -> vec2;
  auto bind_window_space_uniform_buffer() -> void;

protected:
  bool m_padding = true;
  bool m_active = false;
  UniquePtr<ICamera> m_camera;
  SharedPtr<GPUUniformBuffer> m_uniform_buffer;
  SharedPtr<GPUFrameBuffer> m_framebuffer;
  std::unordered_map<KeyCode, std::function<void()>> m_keymaps = {};
  IOperator* m_active_operator = nullptr;
  String m_debug_message = String();
};

} // namespace zod
