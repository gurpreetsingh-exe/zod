#pragma once

#include <glad/glad.h>

#include "gpu/buffer.hh"

namespace zod {

class GLUniformBuffer : public GPUUniformBuffer {
public:
  GLUniformBuffer(usize /* size */);
  ~GLUniformBuffer();

public:
  auto bind(int) -> void override;
  auto unbind() -> void override;
  auto upload_data(const void*, usize, usize) -> void override;
  auto update_data(const void*, usize, usize = 0) -> void override { TODO(); }

private:
  GLuint m_id;
};

class GLVertexBuffer : public GPUVertexBuffer {
public:
  GLVertexBuffer();
  ~GLVertexBuffer();

public:
  auto bind(int = 0) -> void override;
  auto unbind() -> void override;
  auto upload_data(const void*, usize, usize size = 0) -> void override;
  auto update_data(const void*, usize, usize = 0) -> void override;

private:
  GLuint m_id;
};

class GLIndexBuffer : public GPUIndexBuffer {
public:
  GLIndexBuffer();
  ~GLIndexBuffer();

public:
  auto bind(int = 0) -> void override;
  auto unbind() -> void override;
  auto upload_data(const void*, usize, usize size = 0) -> void override;
  auto update_data(const void*, usize, usize = 0) -> void override { TODO(); }

private:
  GLuint m_id;
};

class GLStorageBuffer : public GPUStorageBuffer {
public:
  GLStorageBuffer();
  ~GLStorageBuffer();

public:
  auto bind(int = 0) -> void override;
  auto unbind() -> void override;
  auto upload_data(const void*, usize, usize size = 0) -> void override;
  auto update_data(const void*, usize, usize = 0) -> void override;

private:
  GLuint m_id;
};

} // namespace zod
