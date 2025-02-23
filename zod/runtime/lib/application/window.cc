#include "application/window.hh"
#include "application/input.hh"

namespace zod {

static auto error_callback(int error, const char* description) -> void {
  fmt::println("{}: {}", error, description);
}

Window::Window(const String& name) {
  if (not glfwInit()) {
    eprintln("GLFW initialization failed");
  }

  glfwSetErrorCallback(error_callback);
  // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
#ifdef VULKAN_BACKEND
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

  m_window = glfwCreateWindow(1280, 720, name.c_str(), nullptr, nullptr);
  if (not m_window) {
    eprintln("GLFW window creation failed");
  }

  glfwSetWindowSizeLimits(m_window, 400, 300, GLFW_DONT_CARE, GLFW_DONT_CARE);
#ifdef OPENGL_BACKEND
  glfwMakeContextCurrent(m_window);
#endif

  m_gcx = gpu_context_create(m_window);

  glfwSetCursorPosCallback(m_window, [](GLFWwindow* win, double x, double y) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    auto button = MouseButton::None;
    if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      button = MouseButton::Left;
    } else if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
      button = MouseButton::Right;
    }

    Event event = { .kind = Event::MouseMove,
                    .button = button,
                    .mouse = { f32(x), f32(y) },
                    .last_mouse = g_last_mouse_pos };
    window->m_event_callback(event);
    g_last_mouse_pos = event.mouse;
  });
  glfwSetKeyCallback(m_window, [](GLFWwindow* win, int key, int scancode,
                                  int action, int mods) {
    g_input_state[key] = action == GLFW_PRESS or action == GLFW_REPEAT;
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    auto kind = Event::None;
    if (action == GLFW_PRESS) {
      kind = Event::KeyDown;
    } else if (action == GLFW_RELEASE) {
      kind = Event::KeyUp;
    } else if (action == GLFW_REPEAT) {
      kind = Event::KeyRepeat;
    }
    Event event = { .kind = kind,
                    .shift = bool(mods & GLFW_MOD_SHIFT),
                    .ctrl = bool(mods & GLFW_MOD_CONTROL),
                    .alt = bool(mods & GLFW_MOD_ALT),
                    .key = key };
    window->m_event_callback(event);
  });
  glfwSetMouseButtonCallback(
      m_window, [](GLFWwindow* win, int button, int action, int mods) {
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
                        .button = MouseButton(button),
                        .shift = bool(mods & GLFW_MOD_SHIFT),
                        .ctrl = bool(mods & GLFW_MOD_CONTROL),
                        .alt = bool(mods & GLFW_MOD_ALT),
                        .mouse = { f32(x), f32(y) } };
        window->m_event_callback(event);
      });
  glfwSetWindowSizeCallback(
      m_window, [](GLFWwindow* win, int width, int height) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
        Event event = { .kind = Event::WindowResize,
                        .size = { f32(width), f32(height) } };
        window->m_size = { width, height };
        window->m_event_callback(event);
      });
  glfwSetWindowCloseCallback(m_window, [](GLFWwindow* win) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    Event event = { .kind = Event::WindowClose };
    window->m_event_callback(event);
  });
  glfwSetWindowUserPointer(m_window, this);
}

auto Window::update() -> void {
  glfwPollEvents();
  glfwSwapBuffers(m_window);
}

auto Window::get_mouse_position() -> vec2 {
  f64 x, y;
  glfwGetCursorPos(m_window, &x, &y);
  return { x, y };
}

auto Window::set_mouse_position(vec2 position) -> void {
  glfwSetCursorPos(m_window, position.x, position.y);
}

auto Window::get_window_position() -> vec2 {
  int x, y;
  glfwGetWindowPos(m_window, &x, &y);
  return { x, y };
}

auto Window::set_window_position(vec2) -> void { TODO(); }

auto Window::get_size() -> vec2 {
  int x, y;
  glfwGetFramebufferSize(m_window, &x, &y);
  return { x, y };
}

auto Window::set_vsync(bool vsync) -> void { glfwSwapInterval(vsync); }

Window::~Window() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

} // namespace zod
