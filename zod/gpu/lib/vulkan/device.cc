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
  ZASSERT(not is_initialized());
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

  init_physical_device_properties();
  VKBackend::platform_init(*this);
}

auto VKDevice::init_physical_device_properties() -> void {
  ZASSERT(m_physical_device != VK_NULL_HANDLE);
  vkGetPhysicalDeviceProperties(m_physical_device,
                                &m_physical_device_properties);
}

/// https://www.reddit.com/r/vulkan/comments/4ta9nj/is_there_a_comprehensive_list_of_the_names_and/d5nso2t/
/// https://projects.blender.org/blender/blender/src/commit/ee0fd0ed195cc668eb2552a55a7735ccaf23ff70/source/blender/gpu/vulkan/vk_device.cc#L254-L258
enum PCI : i32 {
  Amd = 0x1002,
  ImgTec = 0x1010,
  Apple = 0x106B,
  Nvidia = 0x10DE,
  Arm = 0x13B5,
  Qualcomm = 0x5143,
  Intel = 0x8086,
};

auto VKDevice::device_type() const -> GPUDeviceType {
  switch (m_physical_device_properties.vendorID) {
    case PCI::Nvidia:
      return GPUDeviceType::Nvidia;
    case PCI::Intel:
      return GPUDeviceType::Intel;
    case PCI::Apple:
      return GPUDeviceType::Apple;
    default:
      return GPUDeviceType::None;
  }
}

auto VKDevice::vendor_name() const -> std::string {
  // Below 0x10000 are the PCI vendor IDs
  // https://pcisig.com/membership/member-companies
  switch (m_physical_device_properties.vendorID) {
    case PCI::Nvidia:
      return "NVIDIA Corporation";
    case PCI::Intel:
      return "Intel Corporation";
    case PCI::Apple:
      return "Apple";
    default:
      return std::to_string(m_physical_device_properties.vendorID);
  }
}

auto VKDevice::driver_version() const -> std::string {
  const u32 driver_version = m_physical_device_properties.driverVersion;
  switch (m_physical_device_properties.vendorID) {
    case PCI::Nvidia:
      return std::to_string((driver_version >> 22) & 0x3FF) + "." +
             std::to_string((driver_version >> 14) & 0xFF) + "." +
             std::to_string((driver_version >> 6) & 0xFF) + "." +
             std::to_string(driver_version & 0x3F);
    case PCI::Intel: {
      const u32 major = VK_VERSION_MAJOR(driver_version);
      if (major > 30) {
        return std::to_string((driver_version >> 14) & 0x3FFFF) + "." +
               std::to_string(driver_version & 0x3FFF);
      }
    } break;
    default:
      break;
  }

  return std::to_string(VK_VERSION_MAJOR(driver_version)) + "." +
         std::to_string(VK_VERSION_MINOR(driver_version)) + "." +
         std::to_string(VK_VERSION_PATCH(driver_version));
}

VKDevice::~VKDevice() { vkDestroySurfaceKHR(m_instance, g_surface, nullptr); }

} // namespace zod
