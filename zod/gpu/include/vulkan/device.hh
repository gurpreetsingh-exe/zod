#pragma once

#include <vulkan/vulkan.h>

namespace zod {

class VKBackend;

class VKDevice {
public:
  auto init(void* /* glfw_window */) -> void;
  auto is_initialized() const -> bool { return m_device != VK_NULL_HANDLE; }

private:
  VkInstance m_instance = VK_NULL_HANDLE;
  VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkQueue m_queue = VK_NULL_HANDLE;
};

} // namespace zod
