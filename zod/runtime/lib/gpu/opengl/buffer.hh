#pragma once

#include <glad/glad.h>

#include "gpu/buffer.hh"

namespace zod {

class GLBuffer : public GPUBuffer {
public:
  GLBuffer(GPUBufferCreateInfo /* info */);
  ~GLBuffer();

public:
  auto bind(int = 0) -> void override;
  auto unbind() -> void override;
  auto write(const void*, usize /* size */, usize /* offset */ = 0)
      -> void override;

private:
  GLuint m_id;
  GLenum m_usage;
  int m_bound_slot = -1;
};

} // namespace zod
