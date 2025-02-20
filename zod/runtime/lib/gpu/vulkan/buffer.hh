#pragma once

#include "./vk_common.hh"
#include "gpu/buffer.hh"

namespace zod {

class VKBuffer {
public:
  VKBuffer() = default;

public:
  auto is_allocated() const -> bool;
  auto is_mapped() const -> bool;
  auto create(usize /* size */, VkBufferUsageFlags) -> void;
  auto destroy() -> void;
  auto mapped_memory() const -> void*;
  auto update(const void*) const -> void;
  auto flush() const -> void;

private:
  auto map() -> void;
  auto unmap() -> void;

private:
  VkBuffer m_buffer = VK_NULL_HANDLE;
  VmaAllocation m_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo m_info;
  void* m_memory_map = nullptr;
  usize m_size;
};

class VKUniformBuffer : public GPUUniformBuffer {
public:
  VKUniformBuffer(usize /* size */);
  ~VKUniformBuffer();

public:
  auto bind(int) -> void override;
  auto unbind() -> void override;
  auto upload_data(const void*, usize, usize) -> void override;
  auto update_data(const void*, usize, usize = 0) -> void override { TODO(); }

private:
  VKBuffer m_buffer;
};

class VKStorageBuffer : public GPUStorageBuffer {
public:
  VKStorageBuffer(usize /* size */);
  ~VKStorageBuffer();

public:
  auto bind(int) -> void override;
  auto unbind() -> void override;
  auto upload_data(const void*, usize, usize) -> void override;
  auto update_data(const void*, usize, usize = 0) -> void override;

private:
  VKBuffer m_buffer;
};

class VKVertexBuffer : public GPUVertexBuffer {
public:
  VKVertexBuffer();
  ~VKVertexBuffer();

public:
  auto bind(int = 0) -> void override;
  auto unbind() -> void override;
  auto upload_data(const void*, usize, usize = 0) -> void override;
  auto update_data(const void*, usize, usize = 0) -> void override;

private:
  VKBuffer m_buffer;
};

} // namespace zod
