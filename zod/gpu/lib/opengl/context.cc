// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "opengl/context.hh"
#include "opengl/framebuffer.hh"

namespace zod {

constexpr int g_titlebar_height = 0;

GLContext::GLContext(void* glfw_window) {
  m_window = glfw_window;
  int w, h;
  glfwGetWindowSize((GLFWwindow*)m_window, &w, &h);
  glViewport(0, 0, w, h - g_titlebar_height);
  glfwSetFramebufferSizeCallback((GLFWwindow*)m_window,
                                 [](GLFWwindow*, int w, int h) {
                                   glViewport(0, 0, w, h - g_titlebar_height);
                                 });
}

GLContext::~GLContext() {}

} // namespace zod
