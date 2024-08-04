#pragma once

#include "buffer.hh"
#include "shader.hh"

namespace zod {

enum class GPUDataType {
  Int,
  Float,
};

inline auto gpu_sizeof(GPUDataType type) -> usize {
  switch (type) {
    case GPUDataType::Int:
    case GPUDataType::Float:
      return 4;
  }
  UNREACHABLE();
}

struct GPUBufferLayout {
  GPUDataType type;
  void* buffer;
  usize size;
  usize length;
  bool instanced = false;
};

class GPUBatch {
protected:
  std::vector<Shared<GPUVertexBuffer>> m_vertex_buffers;
  Shared<GPUIndexBuffer> m_index_buffer;

public:
  virtual ~GPUBatch() = default;

public:
  auto update_binding(usize index, void* data, usize size) -> void {
    m_vertex_buffers[index]->update_data(data, size);
  }

  virtual auto draw(Shared<GPUShader>) -> void = 0;
  virtual auto draw_instanced(Shared<GPUShader>, usize /* instance_count */)
      -> void = 0;
};

} // namespace zod
