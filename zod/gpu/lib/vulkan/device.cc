#include <VkBootstrap.h>

#include "vulkan/backend.hh"
#include "vulkan/device.hh"

namespace zod {

auto VKDevice::init(void* glfw_window) -> void {
  vkb::InstanceBuilder builder;
  auto instance = builder.set_app_name("Zod")
                      .request_validation_layers()
                      .use_default_debug_messenger()
                      .build();
  if (not instance) {
    eprintln("failed to create Vulkan instance\n{}",
             instance.error().message());
  }

  auto vkb_inst = instance.value();
  m_instance = VkInstance(vkb_inst);
  auto selector = vkb::PhysicalDeviceSelector(vkb_inst);
  auto physical_device = selector.set_surface((VkSurfaceKHR)glfw_window)
                             .set_minimum_version(1, 3)
                             .require_dedicated_transfer_queue()
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

} // namespace zod
