#include "window.hh"

namespace zod {

static auto error_callback(int error, const char* description) -> void {
  fmt::println("{}: {}", error, description);
}

auto Window::create(int width, int height, const char* name) -> Unique<Window> {
  auto window = unique<Window>();
  if (not glfwInit()) {
    eprintln("GLFW initialization failed");
  }

  glfwSetErrorCallback(error_callback);
  // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

  GLFWwindow* win = glfwCreateWindow(width, height, name, nullptr, nullptr);
  if (not win) {
    eprintln("GLFW window creation failed");
  }

  glfwMakeContextCurrent(win);

  window->m_window = win;
  window->m_gcx = gpu_context_create(win);
  glfwSetCursorPosCallback(win, [](GLFWwindow* win, double x, double y) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    Event event = { .kind = Event::MouseMove,
                    .button = Event::MouseButtonNone,
                    .mouse = { f32(x), f32(window->m_height - y) } };
    window->m_event_callback(event);
  });
  glfwSetMouseButtonCallback(
      win, [](GLFWwindow* win, int button, int action, int mods) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
        auto kind = Event::None;
        if (action == GLFW_PRESS) {
          kind = Event::MouseDown;
        } else if (action == GLFW_RELEASE) {
          kind = Event::MouseUp;
        }
        Event event = { .kind = kind, .button = (Event::ButtonKind)button };
        window->m_event_callback(event);
      });
  glfwSetWindowSizeCallback(win, [](GLFWwindow* win, int width, int height) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    Event event = { .kind = Event::WindowResize,
                    .size = { f32(width), f32(height) } };
    window->m_width = width;
    window->m_height = height;
    window->m_event_callback(event);
  });
  glfwSetWindowUserPointer(win, window.get());
  return window;
}

Window::~Window() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

} // namespace zod
