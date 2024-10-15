#include <VkBootstrap.h>

#include "vulkan/backend.hh"
#include "vulkan/device.hh"

extern "C" {
auto glfwCreateWindowSurface(VkInstance, void*, const VkAllocationCallbacks*,
                             VkSurfaceKHR*) -> VkResult;
}

namespace zod {

VkSurfaceKHR g_surface;

auto VKDevice::init(void* glfw_window) -> void {
  vkb::InstanceBuilder builder;
  auto instance = builder.set_app_name("Zod")
                      .request_validation_layers()
                      .use_default_debug_messenger()
                      .require_api_version(1, 2, 0)
                      .build();
  if (not instance) {
    eprintln("failed to create Vulkan instance\n{}",
             instance.error().message());
  }

  auto vkb_inst = instance.value();
  m_instance = VkInstance(vkb_inst);
  /// this is so scuffed :(
  if (glfwCreateWindowSurface(m_instance, glfw_window, nullptr, &g_surface) !=
      VK_SUCCESS) {
    eprintln("failed to create Vulkan Surface");
  }

  auto selector = vkb::PhysicalDeviceSelector(vkb_inst);
  auto physical_device =
      selector.set_surface(g_surface)
          .set_minimum_version(1, 2)
          .set_required_features_12(VkPhysicalDeviceVulkan12Features {
              .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
              .descriptorIndexing = true,
              .bufferDeviceAddress = true })
          .select();
  if (not physical_device) {
    eprintln("failed to select Vulkan Physical Device\n{}",
             physical_device.error().message());
  }

  auto vkb_phys = physical_device.value();
  m_physical_device = VkPhysicalDevice(vkb_phys);

  auto device_builder = vkb::DeviceBuilder(vkb_phys);
  auto device = device_builder.build();
  if (not device) {
    eprintln("failed to select Vulkan Device\n{}", device.error().message());
  }

  auto vkb_device = device.value();
  m_device = vkb_device.device;

  auto graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics);
  if (not graphics_queue) {
    eprintln("failed to get graphics queue\n{}",
             graphics_queue.error().message());
  }

  m_queue = graphics_queue.value();

  VKBackend::platform_init(*this);
}

VKDevice::~VKDevice() { vkDestroySurfaceKHR(m_instance, g_surface, nullptr); }

} // namespace zod
