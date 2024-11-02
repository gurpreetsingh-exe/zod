#include "vulkan/buffer.hh"
#include "vulkan/backend.hh"

namespace zod {

auto VKBuffer::is_allocated() const -> bool {
  return m_allocation != VK_NULL_HANDLE;
}

auto VKBuffer::is_mapped() const -> bool { return m_memory_map != nullptr; }

auto VKBuffer::create(usize size, VkBufferUsageFlags usage) -> void {
  ZASSERT(not is_allocated());
  ZASSERT(m_buffer == VK_NULL_HANDLE);
  ZASSERT(m_memory_map == nullptr);
  m_size = size;
  VkBufferCreateInfo info = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  info.pNext = nullptr;
  info.size = size;
  info.usage = usage;

  VmaAllocationCreateInfo vma_info = {};
  vma_info.usage = VMA_MEMORY_USAGE_AUTO;
  vma_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                   VMA_ALLOCATION_CREATE_MAPPED_BIT;

  auto allocator = VKBackend::get().device.mem_allocator();
  VK_CHECK(vmaCreateBuffer(allocator, &info, &vma_info, &m_buffer,
                           &m_allocation, &m_info));
  map();
}

auto VKBuffer::destroy() -> void {
  auto allocator = VKBackend::get().device.mem_allocator();
  vmaDestroyBuffer(allocator, m_buffer, m_allocation);
}

auto VKBuffer::mapped_memory() const -> void* {
  ZASSERT(is_allocated() and is_mapped());
  return m_memory_map;
}

auto VKBuffer::map() -> void {
  ZASSERT(not is_mapped());
  auto allocator = VKBackend::get().device.mem_allocator();
  VK_CHECK(vmaMapMemory(allocator, m_allocation, &m_memory_map));
}

auto VKBuffer::update(const void* data) const -> void {
  ZASSERT(is_mapped(), "Cannot update a non-mapped buffer.");
  std::memcpy(m_memory_map, data, m_size);
  flush();
}

auto VKBuffer::flush() const -> void {
  auto allocator = VKBackend::get().device.mem_allocator();
  vmaFlushAllocation(allocator, m_allocation, 0, m_size);
}

auto VKBuffer::unmap() -> void {
  ZASSERT(is_mapped());
  auto allocator = VKBackend::get().device.mem_allocator();
  vmaUnmapMemory(allocator, m_allocation);
  m_memory_map = nullptr;
}

/// UNIFORM BUFFER ///////////////////////////////////////////
VKUniformBuffer::VKUniformBuffer(usize size)
    : GPUUniformBuffer(size), m_buffer(VKBuffer()) {
  m_buffer.create(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

VKUniformBuffer::~VKUniformBuffer() { m_buffer.destroy(); }

auto VKUniformBuffer::bind(int) -> void { TODO(); }
auto VKUniformBuffer::unbind() -> void { TODO(); }
auto VKUniformBuffer::upload_data(const void*, usize, usize) -> void { TODO(); }

/// STORAGE BUFFER ///////////////////////////////////////////
VKStorageBuffer::VKStorageBuffer(usize size)
    : GPUStorageBuffer(size), m_buffer(VKBuffer()) {
  m_buffer.create(size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

VKStorageBuffer::~VKStorageBuffer() { m_buffer.destroy(); }

auto VKStorageBuffer::bind(int) -> void { TODO(); }
auto VKStorageBuffer::unbind() -> void { TODO(); }
auto VKStorageBuffer::upload_data(const void* data, usize size, usize offset)
    -> void {
  ZASSERT(m_buffer.is_mapped());
  m_buffer.update(data);
}

auto VKStorageBuffer::update_data(const void*, usize, usize) -> void { TODO(); }

/// VERTEX BUFFER ///////////////////////////////////////////
VKVertexBuffer::VKVertexBuffer() : GPUVertexBuffer(), m_buffer(VKBuffer()) {
  m_buffer.create(1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

VKVertexBuffer::~VKVertexBuffer() { m_buffer.destroy(); }

auto VKVertexBuffer::bind(int) -> void { TODO(); }
auto VKVertexBuffer::unbind() -> void { TODO(); }
auto VKVertexBuffer::upload_data(const void* data, usize size, usize offset)
    -> void {
  ZASSERT(m_buffer.is_mapped());
  m_buffer.update(data);
}

auto VKVertexBuffer::update_data(const void*, usize, usize) -> void { TODO(); }

} // namespace zod
