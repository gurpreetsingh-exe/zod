#pragma once

#include <glad/glad.h>

#include "../texture.hh"

namespace zod {

class GLTexture : public GPUTexture {
  friend class GLFrameBuffer;

private:
  GLuint m_id;
  GLenum m_target;
  bool m_bindless;

public:
  GLTexture(GPUTextureType, int w, int h, bool bindless);
  ~GLTexture() = default;

public:
  auto bind() -> void override { glBindTexture(m_target, m_id); }
  auto unbind() -> void override { glBindTexture(m_target, 0); }
  auto resize(i32, i32) -> void override;
  auto get_id() -> void* override { return (void*)(intptr_t)m_id; }
};

} // namespace zod
