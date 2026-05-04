#include "debug.hh"

namespace zod {

namespace gl {

auto get_type(GLenum type) -> GLenum {
  switch (type) {
    case GL_VERTEX_SHADER:
    case GL_FRAGMENT_SHADER:
    case GL_COMPUTE_SHADER:
      return GL_SHADER;
    case GL_ARRAY_BUFFER:
    case GL_ELEMENT_ARRAY_BUFFER:
    case GL_SHADER_STORAGE_BUFFER:
    case GL_UNIFORM_BUFFER:
      return GL_BUFFER;
    case GL_BUFFER:
    case GL_SHADER:
    case GL_TEXTURE:
    case GL_PROGRAM:
    case GL_FRAMEBUFFER:
    case GL_VERTEX_ARRAY:
      return type;
    default:
      ZASSERT(false, "unexpected enum type");
  }
  UNREACHABLE();
}

auto object_label(GLenum type, GLuint id, const char* name) -> void {
  ZASSERT(name);
  glObjectLabel(get_type(type), id, -1, name);
}

} // namespace gl

} // namespace zod
