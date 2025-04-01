#include <glad/glad.h>

#include <set>

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
  auto vendor = String((const char*)glGetString(GL_VENDOR));
  auto version = String((const char*)glGetString(GL_VERSION));
  auto renderer = String((const char*)glGetString(GL_RENDERER));
  auto device = GPUDeviceType::None;
  if (vendor.contains("Intel")) {
    device = GPUDeviceType::Intel;
  } else {
    TODO();
  }

  GLint no_of_extensions = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &no_of_extensions);

  auto extensions = std::set<String>();
  for (int i = 0; i < no_of_extensions; ++i) {
    extensions.insert((const char*)glGetStringi(GL_EXTENSIONS, i));
  }

  if (extensions.find("GL_ARB_bindless_texture") == extensions.end()) {
    fmt::println(stderr, "bindless textures not supported");
  }

  g_platform.init(GPUBackendType::OpenGL, device, std::move(vendor),
                  std::move(version), std::move(renderer));
  // glEnable(GL_DEBUG_OUTPUT);
  // glDebugMessageCallback(message_callback, 0);
}

} // namespace zod
