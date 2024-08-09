#pragma once

#include <GLFW/glfw3.h>

#include "context.hh"
#include "event.hh"

namespace zod {

class Window {
public:
  using EventCallbackFn = std::function<void(Event&)>;
  ~Window();

public:
  static auto create(int width, int height, const char* name) -> Unique<Window>;

  template <typename UpdateCallback>
  auto is_running(UpdateCallback update) -> void {
    while (not glfwWindowShouldClose(m_window)) {
      update();
      glfwPollEvents();
      glfwSwapBuffers(m_window);
    }
  }

  auto set_event_callback(const EventCallbackFn& cb) -> void {
    m_event_callback = cb;
  }

  auto get_mouse_pos() -> std::pair<f32, f32> {
    f64 x, y;
    glfwGetCursorPos(m_window, &x, &y);
    return { x, m_height - y };
  }

  auto get_window_pos() -> std::tuple<int, int> {
    int x, y;
    glfwGetWindowPos(m_window, &x, &y);
    return { x, y };
  }

  auto get_size() -> std::tuple<int, int> {
    int x, y;
    glfwGetFramebufferSize(m_window, &x, &y);
    return { x, y };
  }

  auto get_handle() -> GLFWwindow* { return m_window; }

private:
  GLFWwindow* m_window;
  Shared<GPUContext> m_gcx;
  EventCallbackFn m_event_callback;
  int m_width;
  int m_height;
};

extern Window* g_window;

} // namespace zod
