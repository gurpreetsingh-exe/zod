#pragma once

namespace zod {

class GPUBuffer {
public:
  virtual auto bind(int = 0) -> void = 0;
  virtual auto unbind() -> void = 0;
  virtual auto upload_data(const void*, usize /* size */, usize start = 0)
      -> void = 0;
  virtual auto update_data(const void*, usize /* size */, usize start = 0)
      -> void = 0;
};

class GPUUniformBuffer : public GPUBuffer {
protected:
  usize m_size;

protected:
  GPUUniformBuffer(usize size) : m_size(size) {}

public:
  virtual ~GPUUniformBuffer() = default;
};

class GPUVertexBuffer : public GPUBuffer {
public:
  virtual ~GPUVertexBuffer() = default;
};

class GPUIndexBuffer : public GPUBuffer {
public:
  virtual ~GPUIndexBuffer() = default;
};

class GPUStorageBuffer : public GPUBuffer {
protected:
  usize m_size;

protected:
  GPUStorageBuffer(usize size) : m_size(size) {}

public:
  virtual ~GPUStorageBuffer() = default;
};

} // namespace zod
