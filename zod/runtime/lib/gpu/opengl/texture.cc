#include <stb_image.h>

#include "./texture.hh"
#include "backend.hh"

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
    case GPUTextureFormat::Red:
      return GL_RED;
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
    case GPUTextureFormat::Red:
      return GL_RED;
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
  glBindTexture(m_target, 0);
  stbi_image_free(data);
}

GLTexture::GLTexture(GPUTextureType type, const fs::path& path)
    : GPUTexture(type, GPUTextureFormat::RGBA32F) {
  m_target = to_gl(type);
  glCreateTextures(m_target, 1, &m_id);
  glBindTexture(m_target, m_id);

  i32 channels = 0;
  i32 w = 0;
  i32 h = 0;
  auto data = stbi_loadf(path.c_str(), &w, &h, &channels, 0);
  auto fmt = GL_NONE;
  m_format = GPUTextureFormat::RGBA32F;
  switch (channels) {
    case 3: {
      fmt = GL_RGB;
      m_format = GPUTextureFormat::RGB32F;
    } break;
    case 4: {
      fmt = GL_RGBA;
      m_format = GPUTextureFormat::RGBA32F;
    } break;
    default: {
      fmt::println("{}", channels);
      assert(0);
    }
  }

  switch (m_target) {
    case GL_TEXTURE_2D: {
      glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(m_target, 0, to_gl(m_format), w, h, 0, fmt, GL_FLOAT, data);
    } break;
    case GL_TEXTURE_CUBE_MAP: {
      m_width = 1000;
      m_height = 1000;
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      for (u32 i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, to_gl(m_format),
                     1000, 1000, 0, fmt, GL_FLOAT, nullptr);
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

      if (not data) {
        fmt::println("data not found");
        return;
      }

      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
      auto shader = GPUBackend::get().create_shader(
          GPUShaderCreateInfo("panorama_to_cubemap")
              .vertex_source(g_fullscreen_src)
              .fragment_source(g_panorama_to_cubemap_frag_src));
      GLuint hdri;
      glCreateTextures(GL_TEXTURE_2D, 1, &hdri);
      glBindTexture(GL_TEXTURE_2D, hdri);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, to_gl(m_format), w, h, 0, fmt, GL_FLOAT,
                   data);

      u32 fbo;
      glGenFramebuffers(1, &fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      bind();
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_id, 0);
      unbind();

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fmt::println("framebuffer status not complete\n");
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      for (u32 i = 0; i < 6; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        u32 side = i;
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, m_id, 0);

        GLint view[4];
        glGetIntegerv(GL_VIEWPORT, view);
        glViewport(0, 0, i32(m_width), i32(m_height));

        glBindTexture(GL_TEXTURE_2D, hdri);
        shader->bind();
        shader->uniform_uint("u_panorama", ADDR(0U));
        shader->uniform_int("u_current_face", (i32*)&i);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(view[0], view[1], view[2], view[3]);
      }
      glDeleteFramebuffers(1, &fbo);
    } break;
    default:
      assert(false && "Not implemented");
  }
  stbi_image_free(data);
}

auto GLTexture::blit(f32 x, f32 y, f32 width, f32 height, const void* pixels)
    -> void {
  glTexSubImage2D(m_target, 0, x, y, width, height, gl_format(m_format),
                  GL_UNSIGNED_BYTE, pixels);
}

auto GLTexture::resize(i32 width, i32 height) -> void {
  glDeleteTextures(1, &m_id);
  *this = GLTexture(m_type, m_format, width, height, m_bindless);
}

} // namespace zod
