#include "opengl/texture.hh"

namespace zod {

static auto to_gl(GPUTextureType type) -> GLenum {
  switch (type) {
    case GPUTextureType::Texture1D:
      return GL_TEXTURE_1D;
    case GPUTextureType::Texture2D:
      return GL_TEXTURE_2D;
    case GPUTextureType::Texture3D:
      return GL_TEXTURE_3D;
    case GPUTextureType::TextureCube:
      return GL_TEXTURE_CUBE_MAP;
    default:
      ZASSERT(false, "texture type not found");
      UNREACHABLE();
  }
}

GLTexture::GLTexture(GPUTextureType type, int w, int h, bool bindless)
    : GPUTexture(type, w, h), m_bindless(bindless) {
  m_target = to_gl(type);
  glCreateTextures(m_target, 1, &m_id);
  glBindTexture(m_target, m_id);
  glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(m_target, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  glBindTexture(m_target, 0);
}

auto GLTexture::resize(i32 width, i32 height) -> void {
  glDeleteTextures(1, &m_id);
  *this = GLTexture(m_type, width, height, m_bindless);
}

} // namespace zod
