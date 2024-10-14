#include <glad/glad.h>

#include "opengl/state.hh"

namespace zod {

static auto to_gl(Depth depth) -> GLenum {
  switch (depth) {
    case Depth::None:
      return GL_NONE;
    case Depth::Less:
      return GL_LESS;
    case Depth::LessEqual:
      return GL_LEQUAL;
    case Depth::Equal:
      return GL_EQUAL;
    case Depth::Greater:
      return GL_GREATER;
    case Depth::GreaterEqual:
      return GL_GEQUAL;
    case Depth::Always:
    default:
      return GL_ALWAYS;
  }
}

GLState::GLState() {
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

auto GLState::set_depth_test(Depth depth) -> void {
  if (depth != Depth::None) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(to_gl(depth));
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

auto GLState::set_blend(Blend blend) -> void {
  GLenum src;
  GLenum dst;
  switch (blend) {
    case Blend::None: {
      src = GL_NONE;
      dst = GL_NONE;
    } break;
    case Blend::Alpha: {
      src = GL_SRC_ALPHA;
      dst = GL_ONE_MINUS_SRC_ALPHA;
    } break;
  }

  if (blend != Blend::None) {
    glEnable(GL_BLEND);
    glBlendFunc(src, dst);
  } else {
    glDisable(GL_BLEND);
  }
}

auto GLState::draw_immediate(usize n) -> void {
  glBindVertexArray(0);
  glDrawArrays(GL_TRIANGLES, 0, n);
}

} // namespace zod
