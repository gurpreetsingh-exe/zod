#include "./buffer.hh"

namespace zod {

auto to_gl(GPUBufferUsage usage) -> GLenum {
  switch (usage) {
    case GPUBufferUsage::Vertex:
      return GL_ARRAY_BUFFER;
    case GPUBufferUsage::Index:
      return GL_ELEMENT_ARRAY_BUFFER;
    case GPUBufferUsage::Uniform:
      return GL_UNIFORM_BUFFER;
    case GPUBufferUsage::Storage:
      return GL_SHADER_STORAGE_BUFFER;
  }
}

GLBuffer::GLBuffer(GPUBufferCreateInfo info)
    : GPUBuffer(info), m_usage(to_gl(info.usage)) {
  glCreateBuffers(1, &m_id);
  // TODO: immutable buffers
  glNamedBufferData(m_id, info.size, nullptr, GL_STREAM_DRAW);
  if (m_info.usage == GPUBufferUsage::Storage or
      m_info.usage == GPUBufferUsage::Uniform) {
    glBindBufferRange(m_usage, 0, m_id, 0, info.size);
  }
}

GLBuffer::~GLBuffer() { glDeleteBuffers(1, &m_id); }

auto GLBuffer::bind(int slot) -> void {
  if (m_info.usage == GPUBufferUsage::Storage or
      m_info.usage == GPUBufferUsage::Uniform) {
    ZASSERT(m_bound_slot == -1, "buffer already bound at slot `{}`",
            m_bound_slot);
    m_bound_slot = slot;
    glBindBufferBase(m_usage, slot, m_id);
    return;
  }
  glBindBuffer(m_usage, m_id);
}

auto GLBuffer::unbind() -> void {
  if (m_info.usage == GPUBufferUsage::Storage or
      m_info.usage == GPUBufferUsage::Uniform) {
    glBindBufferBase(m_usage, m_bound_slot, 0);
    return;
  }
  glBindBuffer(m_usage, 0);
}

auto GLBuffer::write(const void* data, usize size, usize offset) -> void {
  ZASSERT(m_info.size >= offset + size,
          "`{}`: size `{}` exceeds buffer size `{}`", m_info.name, size,
          m_info.size);
  glNamedBufferSubData(m_id, offset, size, data);
}

} // namespace zod
