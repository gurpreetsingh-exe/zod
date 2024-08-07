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

  glfwSetWindowSizeLimits(win, 400, 300, GLFW_DONT_CARE, GLFW_DONT_CARE);
  glfwMakeContextCurrent(win);

  window->m_window = win;
  window->m_gcx = gpu_context_create(win);

  glfwSetCursorPosCallback(win, [](GLFWwindow* win, double x, double y) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    auto button = Event::MouseButtonNone;
    if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      button = Event::MouseButtonLeft;
    } else if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
      button = Event::MouseButtonRight;
    }

    Event event = { .kind = Event::MouseMove,
                    .button = button,
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
        f64 x, y;
        glfwGetCursorPos(win, &x, &y);
        Event event = { .kind = kind,
                        .button = (Event::ButtonKind)button,
                        .shift = bool(mods & GLFW_MOD_SHIFT),
                        .ctrl = bool(mods & GLFW_MOD_CONTROL),
                        .alt = bool(mods & GLFW_MOD_ALT),
                        .mouse = { f32(x), window->m_height - f32(y) } };
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
