#include <stb_image.h>

#include "./texture.hh"
#include "backend.hh"

#include "gpu/shader_builtins.hh"

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
    case GPUTextureType::TextureArray:
      return GL_TEXTURE_2D_ARRAY;
    default:
      ZASSERT(false, "texture type not found");
      UNREACHABLE();
  }
}

static auto to_gl(GPUTextureFormat format) -> GLenum {
  switch (format) {
    case GPUTextureFormat::RGBA8:
      return GL_RGBA8;
    case GPUTextureFormat::RGB8:
      return GL_RGB8;
    case GPUTextureFormat::RGBA32:
      return GL_RGBA32UI;
    case GPUTextureFormat::RG32F:
      return GL_RG32F;
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

static auto to_gl(GPUTextureData data) -> GLenum {
  switch (data) {
    case GPUTextureData::UByte:
      return GL_UNSIGNED_BYTE;
    case GPUTextureData::Float:
      return GL_FLOAT;
  }
  UNREACHABLE();
};

static auto gl_format(GPUTextureFormat format) -> GLenum {
  switch (format) {
    case GPUTextureFormat::RGBA8:
    case GPUTextureFormat::RGBA32:
    case GPUTextureFormat::RGBA32F:
      return GL_RGBA;
    case GPUTextureFormat::RGB8:
    case GPUTextureFormat::RGB32F:
      return GL_RGB;
    case GPUTextureFormat::R32UI:
      return GL_RED_INTEGER;
    case GPUTextureFormat::Red:
      return GL_RED;
    case GPUTextureFormat::RG32F:
      return GL_RG;
    default:
      ZASSERT(false, "texture format not found");
      UNREACHABLE();
  }
}

auto to_gl(GPUTextureWrap wrap) -> GLenum {
  switch (wrap) {
    case GPUTextureWrap::Repeat:
      return GL_REPEAT;
    case GPUTextureWrap::Clamp:
      return GL_CLAMP_TO_EDGE;
    default:
      UNREACHABLE();
  }
}

GLTexture::GLTexture(GPUTextureCreateInfo info) : GPUTexture(info) {
  if (m_info.type == GPUTextureType::TextureCube) {
    ZASSERT(not m_info.path.empty());
    create_cubemap();
    return;
  }

  if (not m_info.path.empty()) {
    from_path();
  }
  m_target = to_gl(m_info.type);
  glCreateTextures(m_target, 1, &m_id);
  glBindTexture(m_target, m_id);
  glTexParameteri(m_target, GL_TEXTURE_WRAP_S, to_gl(m_info.wrap));
  glTexParameteri(m_target, GL_TEXTURE_WRAP_T, to_gl(m_info.wrap));
  glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER,
                  m_info.mips ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR);
  glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  if (m_info.type == GPUTextureType::TextureArray) {
    glTexImage3D(m_target, 0, to_gl(m_info.format), m_info.width, m_info.height,
                 m_info.layers, 0, gl_format(m_info.format), to_gl(m_info.data),
                 m_info.pixels);
  } else {
    glTexImage2D(m_target, 0, to_gl(m_info.format), m_info.width, m_info.height,
                 0, gl_format(m_info.format), to_gl(m_info.data),
                 m_info.pixels);
  }
  generate_mipmap();
  glBindTexture(m_target, 0);
}

template <typename T>
auto GLTexture::copy_texture_data(i32 width, i32 height, T* data) -> void {
  m_info.pixels = new T[width * height * 4];
  for (usize x = 0; x < width; ++x) {
    for (usize y = 0; y < height; ++y) {
      auto index = x + y * width;
      auto src = &data[index * 2];
      auto dst = &((T*)m_info.pixels)[index * 4];
      dst[0] = src[1];
      dst[1] = src[1];
      dst[2] = src[1];
      dst[3] = src[0];
    }
  }
}

auto GLTexture::from_path() -> void {
  i32 channels = 0;
  stbi_set_flip_vertically_on_load(true);
  if (m_info.data == GPUTextureData::Float) {
    auto* data = stbi_loadf(m_info.path.c_str(), &m_info.width, &m_info.height,
                            &channels, 0);
    switch (channels) {
      case 2: {
        copy_texture_data<f32>(m_info.width, m_info.height, data);
        stbi_image_free(data);
      } break;
      case 4: {
        m_info.pixels = new f32[m_info.width * m_info.height * 4];
        std::memcpy(m_info.pixels, data,
                    m_info.width * m_info.height * 4 * sizeof(f32));
        stbi_image_free(data);
      } break;
      default: {
        fmt::println("{}", channels);
        ZASSERT(false);
      }
    }
    m_info.format = GPUTextureFormat::RGBA32F;
  } else {
    auto* data = stbi_load(m_info.path.c_str(), &m_info.width, &m_info.height,
                           &channels, 0);
    switch (channels) {
      case 2: {
        copy_texture_data<u8>(m_info.width, m_info.height, data);
        stbi_image_free(data);
      } break;
      case 4: {
        auto width = m_info.width;
        auto height = m_info.height;
        m_info.pixels = new u8[width * height * 4];
        for (usize x = 0; x < width; ++x) {
          for (usize y = 0; y < height; ++y) {
            auto index = x + y * width;
            auto src = &data[index * 4];
            auto dst = &((u8*)m_info.pixels)[index * 4];
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = src[3];
          }
        }
      } break;
      default: {
        fmt::println("{}", channels);
        ZASSERT(false);
      }
    }
    m_info.format = GPUTextureFormat::RGBA8;
  }
}

auto GLTexture::create_cubemap() -> void {
  m_target = to_gl(m_info.type);
  glCreateTextures(m_target, 1, &m_id);
  glBindTexture(m_target, m_id);

  i32 channels = 0;
  i32 w = 0;
  i32 h = 0;
  auto data = stbi_loadf(m_info.path.c_str(), &w, &h, &channels, 0);
  auto fmt = GL_NONE;
  m_info.format = GPUTextureFormat::RGBA32F;
  switch (channels) {
    case 3: {
      fmt = GL_RGB;
      m_info.format = GPUTextureFormat::RGB32F;
    } break;
    case 4: {
      fmt = GL_RGBA;
      m_info.format = GPUTextureFormat::RGBA32F;
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
      glTexImage2D(m_target, 0, to_gl(m_info.format), w, h, 0, fmt, GL_FLOAT,
                   data);
    } break;
    case GL_TEXTURE_CUBE_MAP: {
      m_info.width = 1000;
      m_info.height = 1000;
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      for (u32 i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                     to_gl(m_info.format), 1000, 1000, 0, fmt, GL_FLOAT,
                     nullptr);
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
      glTexImage2D(GL_TEXTURE_2D, 0, to_gl(m_info.format), w, h, 0, fmt,
                   GL_FLOAT, data);

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
        glViewport(0, 0, i32(m_info.width), i32(m_info.height));

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

auto GLTexture::blit(f32 x, f32 y, f32 width, f32 height, const void* pixels,
                     usize depth) -> void {
  if (m_info.type == GPUTextureType::TextureArray) {
    glTexSubImage3D(m_target, 0, x, y, depth, width, height, 1,
                    gl_format(m_info.format), to_gl(m_info.data), pixels);
  } else {
    glTexSubImage2D(m_target, 0, x, y, width, height, gl_format(m_info.format),
                    to_gl(m_info.data), pixels);
  }
}

auto GLTexture::generate_mipmap() -> void {
  if (m_info.mips) {
    glTexParameteri(m_target, GL_TEXTURE_MAX_LOD, m_info.mips);
    glGenerateMipmap(m_target);
  }
}

auto GLTexture::resize(i32 width, i32 height) -> void {
  glDeleteTextures(1, &m_id);
  m_info.width = width;
  m_info.height = height;
  *this = GLTexture(m_info);
}

} // namespace zod
