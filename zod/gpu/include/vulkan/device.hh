#pragma once

#include <vulkan/vulkan.h>

#include "platform.hh"

namespace zod {

class VKBackend;

class VKDevice {
public:
  ~VKDevice();

public:
  auto init(void* /* glfw_window */) -> void;
  auto is_initialized() const -> bool { return m_device != VK_NULL_HANDLE; }
  auto init_swapchain(void*) -> void;
  auto device_type() const -> GPUDeviceType;
  auto vendor_name() const -> std::string;
  auto driver_version() const -> std::string;

  auto get_physical_device_properties() const
      -> const VkPhysicalDeviceProperties& {
    return m_physical_device_properties;
  }

private:
  auto init_physical_device_properties() -> void;

private:
  VkInstance m_instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT m_debug_messenger;
  VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkQueue m_queue = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties m_physical_device_properties = {};

  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  VkFormat m_swapchain_image_format = VK_FORMAT_UNDEFINED;
  std::vector<VkImage> m_swapchain_images = {};
  std::vector<VkImageView> m_swapchain_image_views = {};
  VkExtent2D m_swapchain_extent = {};
};

} // namespace zod
