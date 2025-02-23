#pragma once

#include "buffer.hh"
#include "shader.hh"
#include "types.hh"

namespace zod {

struct GPUBufferLayout {
  GPUDataType type;
  void* buffer;
  usize size;
  usize length;
  bool normalized = false;
  bool instanced = false;
};

class GPUBatch {
protected:
  Vector<SharedPtr<GPUVertexBuffer>> m_vertex_buffers;
  SharedPtr<GPUIndexBuffer> m_index_buffer;

public:
  virtual ~GPUBatch() = default;

public:
  auto get_buffer(usize i) -> SharedPtr<GPUVertexBuffer> {
    return m_vertex_buffers[i];
  }

  auto get_index_buffer() -> SharedPtr<GPUIndexBuffer> {
    return m_index_buffer;
  }

  auto update_binding(usize index, void* data, usize size) -> void {
    m_vertex_buffers[index]->update_data(data, size);
  }

  virtual auto draw(SharedPtr<GPUShader>) -> void = 0;
  virtual auto draw(SharedPtr<GPUShader>, usize) -> void = 0;
  virtual auto draw_instanced(SharedPtr<GPUShader>, usize /* instance_count */)
      -> void = 0;
  virtual auto draw_indirect(SharedPtr<GPUShader>) -> void = 0;
  virtual auto upload_indirect(const void*, usize) -> void = 0;
  virtual auto draw_lines(SharedPtr<GPUShader>) -> void = 0;
  virtual auto draw_lines(SharedPtr<GPUShader>, usize) -> void = 0;
};

} // namespace zod
