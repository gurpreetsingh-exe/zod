#pragma once

#include "context.hh"
#include "widget.hh"
#include <GLFW/glfw3.h>

namespace zod {

class SWindow : public SWidget {
public:
  using EventCallbackFn = std::function<void(Event&)>;

  SWindow(std::string name) : SWidget(std::move(name)) {}
  ~SWindow();
  static auto create(std::string) -> Unique<SWindow>;

public:
  auto on_event(Event&) -> void override;
  auto compute_desired_size() -> void override;
  auto draw_imp(Geometry&) -> void override;

public:
  auto set_event_callback(const EventCallbackFn& cb) -> void {
    m_event_callback = cb;
  }

  auto get_mouse_position() -> vec2;
  auto set_mouse_position(vec2) -> void;
  auto get_window_position() -> vec2;
  auto set_window_position(vec2) -> void;
  auto get_size() -> vec2;
  auto set_vsync(bool) -> void;

  template <typename UpdateCallback>
  auto is_running(UpdateCallback update) -> void {
    while (not glfwWindowShouldClose(m_window)) {
      update();
      glfwPollEvents();
      glfwSwapBuffers(m_window);
    }
  }

  auto get_native_handle() -> void* { return m_window; }
  auto close() -> void;

private:
  GLFWwindow* m_window;
  Shared<GPUContext> m_gcx;
  EventCallbackFn m_event_callback;
};

} // namespace zod
