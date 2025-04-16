#pragma once

#include <glad/glad.h>

#include "gpu/texture.hh"

namespace zod {

class GLTexture : public GPUTexture {
  friend class GLFrameBuffer;

private:
  GLuint m_id;
  GLenum m_target;

public:
  GLTexture(GPUTextureCreateInfo);
  GLTexture(GPUTextureType, GPUTextureFormat, const fs::path&);
  GLTexture(GPUTextureType, const fs::path&);
  ~GLTexture() = default;

public:
  auto bind(usize slot = 0) -> void override {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(m_target, m_id);
  }
  auto unbind() -> void override { glBindTexture(m_target, 0); }
  auto resize(i32, i32) -> void override;
  auto get_id() -> void* override { return (void*)(intptr_t)m_id; }
  auto blit(f32, f32, f32, f32, const void* /* pixels */, usize /* layers */)
      -> void override;
  auto generate_mipmap() -> void override;

private:
  auto from_path() -> void;
  auto create_cubemap() -> void;

  template <typename T>
  auto copy_texture_data(i32, i32, T*) -> void;
};

} // namespace zod
