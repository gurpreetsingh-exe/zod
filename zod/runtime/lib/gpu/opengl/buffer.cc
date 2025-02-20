#include "./buffer.hh"

namespace zod {

GLUniformBuffer::GLUniformBuffer(usize size) : GPUUniformBuffer(size) {
  glCreateBuffers(1, &m_id);
  glBindBuffer(GL_UNIFORM_BUFFER, m_id);
  glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_id, 0, size);
}

GLUniformBuffer::~GLUniformBuffer() { glDeleteBuffers(1, &m_id); }

auto GLUniformBuffer::bind(int slot) -> void {
  glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_id);
}

auto GLUniformBuffer::unbind() -> void { glBindBuffer(GL_UNIFORM_BUFFER, 0); }

auto GLUniformBuffer::upload_data(const void* data, usize size, usize start)
    -> void {
  glBindBuffer(GL_UNIFORM_BUFFER, m_id);
  glBufferSubData(GL_UNIFORM_BUFFER, start, size, data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/// Vertex Buffer
GLVertexBuffer::GLVertexBuffer() {
  glCreateBuffers(1, &m_id);
  glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

GLVertexBuffer::~GLVertexBuffer() { glDeleteBuffers(1, &m_id); }

auto GLVertexBuffer::bind(int) -> void { glBindBuffer(GL_ARRAY_BUFFER, m_id); }

auto GLVertexBuffer::unbind() -> void { glBindBuffer(GL_ARRAY_BUFFER, 0); }

auto GLVertexBuffer::upload_data(const void* data, usize size, usize) -> void {
  glBindBuffer(GL_ARRAY_BUFFER, m_id);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

auto GLVertexBuffer::update_data(const void* data, usize size, usize) -> void {
  glBindBuffer(GL_ARRAY_BUFFER, m_id);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

/// Index Buffer
GLIndexBuffer::GLIndexBuffer() {
  glCreateBuffers(1, &m_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

GLIndexBuffer::~GLIndexBuffer() { glDeleteBuffers(1, &m_id); }

auto GLIndexBuffer::bind(int) -> void {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

auto GLIndexBuffer::unbind() -> void {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

auto GLIndexBuffer::upload_data(const void* data, usize size, usize) -> void {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

/// Storage Buffer
GLStorageBuffer::GLStorageBuffer() {
  glCreateBuffers(1, &m_id);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
}

GLStorageBuffer::~GLStorageBuffer() { glDeleteBuffers(1, &m_id); }

auto GLStorageBuffer::bind(int slot) -> void {
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_id);
}

auto GLStorageBuffer::unbind() -> void {
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

auto GLStorageBuffer::upload_data(const void* data, usize size, usize) -> void {
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_id, 0, size);
}

auto GLStorageBuffer::update_data(const void* data, usize size, usize offset)
    -> void {
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

} // namespace zod
