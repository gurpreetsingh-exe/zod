#include <stb_image.h>

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

static auto to_gl(GPUTextureFormat format) -> GLenum {
  switch (format) {
    case GPUTextureFormat::RGBA8:
      return GL_RGBA8;
    case GPUTextureFormat::RGB32F:
      return GL_RGB32F;
    case GPUTextureFormat::RGBA32F:
      return GL_RGBA32F;
    case GPUTextureFormat::R32UI:
      return GL_R32UI;
    default:
      ZASSERT(false, "texture format not found");
      UNREACHABLE();
  }
}

static auto gl_format(GPUTextureFormat format) -> GLenum {
  switch (format) {
    case GPUTextureFormat::RGBA8:
    case GPUTextureFormat::RGB32F:
    case GPUTextureFormat::RGBA32F:
      return GL_RGBA;
    case GPUTextureFormat::R32UI:
      return GL_RED_INTEGER;
    default:
      ZASSERT(false, "texture format not found");
      UNREACHABLE();
  }
}

GLTexture::GLTexture(GPUTextureType type, GPUTextureFormat format, int w, int h,
                     bool bindless)
    : GPUTexture(type, format, w, h), m_bindless(bindless) {
  m_target = to_gl(type);
  glCreateTextures(m_target, 1, &m_id);
  glBindTexture(m_target, m_id);
  glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(m_target, 0, to_gl(format), m_width, m_height, 0,
               gl_format(format), GL_UNSIGNED_BYTE, nullptr);
  glGenerateMipmap(m_target);
  glBindTexture(m_target, 0);
}

GLTexture::GLTexture(GPUTextureType type, GPUTextureFormat format,
                     const fs::path& path)
    : GPUTexture(type, format) {
  i32 channels = 0;
  auto data = stbi_loadf(path.c_str(), &m_width, &m_height, &channels, 0);
  switch (channels) {
    case 2: {
      auto buf = new f32[m_width * m_height * 4];
      for (usize x = 0; x < m_width; ++x) {
        for (usize y = 0; y < m_height; ++y) {
          auto index = x + y * m_width;
          auto src = &data[index * 2];
          auto dst = &buf[index * 4];
          dst[0] = src[1];
          dst[1] = src[1];
          dst[2] = src[1];
          dst[3] = src[0];
        }
      }
      stbi_image_free(data);
      data = buf;
    } break;
    default: {
      fmt::println("{}", channels);
      ZASSERT(false);
    }
  }

  m_target = to_gl(type);
  glCreateTextures(m_target, 1, &m_id);
  glBindTexture(m_target, m_id);
  glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(m_target, 0, to_gl(format), m_width, m_height, 0, GL_RGBA,
               GL_FLOAT, data);
  glGenerateMipmap(m_target);
  glBindTexture(m_target, 0);
  stbi_image_free(data);
}

auto GLTexture::resize(i32 width, i32 height) -> void {
  glDeleteTextures(1, &m_id);
  *this = GLTexture(m_type, m_format, width, height, m_bindless);
}

} // namespace zod
