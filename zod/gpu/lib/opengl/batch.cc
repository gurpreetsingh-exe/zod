#include "opengl/batch.hh"
#include "opengl/buffer.hh"

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

GLBatch::GLBatch(const std::vector<GPUBufferLayout>& layouts,
                 const std::vector<u32>& indices) {
  m_elements = indices.size();
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

  auto buffer = shared<GLIndexBuffer>();
  buffer->bind();
  buffer->upload_data(indices.data(), indices.size() * sizeof(u32));
  m_index_buffer = buffer;
}

GLBatch::~GLBatch() { glDeleteVertexArrays(1, &m_id); }

auto GLBatch::draw(Shared<GPUShader> shader) -> void {
  glBindVertexArray(m_id);
  glDrawElements(GL_TRIANGLES, m_elements, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

auto GLBatch::draw_instanced(Shared<GPUShader> shader, usize instance_count)
    -> void {
  glBindVertexArray(m_id);
  glDrawElementsInstanced(GL_TRIANGLES, m_elements, GL_UNSIGNED_INT, nullptr,
                          instance_count);
  glBindVertexArray(0);
}

} // namespace zod
