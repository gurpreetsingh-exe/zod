#include <glad/glad.h>

#include "opengl/renderer.hh"

namespace zod {

auto GLRenderer::clear_color(const vec4& color) -> void {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(color.r, color.g, color.b, color.a);
}

auto GLRenderer::resize(int width, int height) -> void {
  glViewport(0, 0, width, height);
}

} // namespace zod
