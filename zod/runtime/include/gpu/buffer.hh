#pragma once

namespace zod {

enum class GPUBufferUsage {
  Vertex,
  Index,
  Storage,
  Uniform,
};

struct GPUBufferCreateInfo {
  const char* name;
  GPUBufferUsage usage;
  usize size;
};

class GPUBuffer {
protected:
  GPUBufferCreateInfo m_info;
  GPUBuffer(GPUBufferCreateInfo info) : m_info(info) {}

public:
  virtual ~GPUBuffer() = default;
  virtual auto bind(int = 0) -> void = 0;
  virtual auto unbind() -> void = 0;
  virtual auto write(const void*, usize /* size */, usize /* offset */ = 0)
      -> void = 0;
};

} // namespace zod
