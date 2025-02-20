#include <glad/glad.h>

#include "../platform_private.hh"
#include "./backend.hh"

typedef void (*GLFWglproc)(void);
extern "C" GLFWglproc glfwGetProcAddress(const char*);

namespace zod {

static void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id,
                                        GLenum severity, GLsizei length,
                                        const GLchar* message,
                                        const void* userParam) {
  (void)source;
  (void)id;
  (void)length;
  (void)userParam;
  std::fprintf(stderr, "%stype = 0x%x, severity = 0x%x, message = %s\n",
               (type == GL_DEBUG_TYPE_ERROR ? "error: " : ""), type, severity,
               message);
}

auto GLBackend::platform_init() -> void {
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  auto vendor = std::string((const char*)glGetString(GL_VENDOR));
  auto version = std::string((const char*)glGetString(GL_VERSION));
  auto renderer = std::string((const char*)glGetString(GL_RENDERER));
  auto device = GPUDeviceType::None;
  if (vendor.contains("Intel")) {
    device = GPUDeviceType::Intel;
  } else {
    TODO();
  }
  g_platform.init(GPUBackendType::OpenGL, device, std::move(vendor),
                  std::move(version), std::move(renderer));
  // glEnable(GL_DEBUG_OUTPUT);
  // glDebugMessageCallback(message_callback, 0);
}

} // namespace zod
