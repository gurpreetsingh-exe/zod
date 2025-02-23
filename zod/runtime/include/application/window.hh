#pragma once

#include "application/event.hh"
#include "gpu/context.hh"
#include <GLFW/glfw3.h>

namespace zod {

class Window {
public:
  using EventCallbackFn = std::function<void(Event&)>;

  Window(const String& /* name */);
  ~Window();

public:
  auto set_event_callback(const EventCallbackFn& cb) -> void {
    m_event_callback = cb;
  }

  auto update() -> void;
  auto get_mouse_position() -> vec2;
  auto set_mouse_position(vec2) -> void;
  auto get_window_position() -> vec2;
  auto set_window_position(vec2) -> void;
  auto get_size() -> vec2;
  auto set_vsync(bool) -> void;
  auto get_native_handle() -> void* { return m_window; }

private:
  GLFWwindow* m_window;
  SharedPtr<GPUContext> m_gcx;
  EventCallbackFn m_event_callback;
  vec<2, i32> m_size;
};

} // namespace zod
