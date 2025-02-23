#include "./batch.hh"
#include "./buffer.hh"

namespace zod {

static auto to_gl(GPUDataType type) -> GLenum {
  switch (type) {
    case GPUDataType::Int:
      return GL_INT;
    case GPUDataType::Float:
      return GL_FLOAT;
  }
  UNREACHABLE();
}

GLBatch::GLBatch(const Vector<GPUBufferLayout>& layouts,
                 const Vector<u32>& indices) {
  m_elements =
      indices.size() ? indices.size() : layouts[0].length / layouts[0].size;
  glCreateVertexArrays(1, &m_id);
  glBindVertexArray(m_id);

  for (const auto& layout : layouts) {
    auto buffer = shared<GLVertexBuffer>();
    buffer->bind();
    auto byte_size = gpu_sizeof(layout.type);
    buffer->upload_data(layout.buffer, byte_size * layout.length);
    auto attr = m_vertex_buffers.size();
    glEnableVertexAttribArray(attr);
    glVertexAttribPointer(attr, layout.size, to_gl(layout.type),
                          layout.normalized, byte_size * layout.size, nullptr);
    if (layout.instanced) {
      glVertexBindingDivisor(attr, 1);
    }
    buffer->unbind();
    m_vertex_buffers.push_back(buffer);
  }

  if (indices.empty()) {
    glBindVertexArray(0);
    return;
  }
  auto buffer = shared<GLIndexBuffer>();
  buffer->bind();
  buffer->upload_data(indices.data(), indices.size() * sizeof(u32));
  m_index_buffer = buffer;
  glBindVertexArray(0);
}

GLBatch::~GLBatch() { glDeleteVertexArrays(1, &m_id); }

auto GLBatch::draw(SharedPtr<GPUShader> shader) -> void {
  glBindVertexArray(m_id);
  glDrawElements(GL_TRIANGLES, m_elements, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

auto GLBatch::draw(SharedPtr<GPUShader> shader, usize n) -> void {
  glBindVertexArray(m_id);
  glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

auto GLBatch::draw_instanced(SharedPtr<GPUShader> shader, usize instance_count)
    -> void {
  glBindVertexArray(m_id);
  glDrawElementsInstanced(GL_TRIANGLES, m_elements, GL_UNSIGNED_INT, nullptr,
                          instance_count);
  glBindVertexArray(0);
}

auto GLBatch::draw_indirect(SharedPtr<GPUShader> shader) -> void {
  glBindVertexArray(m_id);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect);
  glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_indirect_size,
                              0);
  glBindVertexArray(0);
}

auto GLBatch::upload_indirect(const void* buffer, usize size) -> void {
  glCreateBuffers(1, &m_indirect);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect);
  glBufferData(GL_DRAW_INDIRECT_BUFFER, size, buffer, GL_DYNAMIC_DRAW);
  m_indirect_size = size / 20;
}

auto GLBatch::draw_lines(SharedPtr<GPUShader> shader) -> void {
  glBindVertexArray(m_id);
  glDrawArrays(GL_LINES, 0, m_elements);
  glBindVertexArray(0);
}

auto GLBatch::draw_lines(SharedPtr<GPUShader> shader, usize n) -> void {
  glBindVertexArray(m_id);
  glDrawArrays(GL_LINES, 0, n);
  glBindVertexArray(0);
}

} // namespace zod
