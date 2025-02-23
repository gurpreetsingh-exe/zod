#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "./backend.hh"
#include "./device.hh"

extern "C" {
auto glfwCreateWindowSurface(VkInstance, void*, const VkAllocationCallbacks*,
                             VkSurfaceKHR*) -> VkResult;
auto glfwGetWindowSize(void*, int*, int*) -> void;
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
  m_debug_messenger = vkb_inst.debug_messenger;
  /// this is so scuffed :(
  VK_CHECK(
      glfwCreateWindowSurface(m_instance, glfw_window, nullptr, &g_surface));

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
  init_swapchain(glfw_window);
  init_allocator();
  VKBackend::platform_init(*this);
}

auto VKDevice::init_physical_device_properties() -> void {
  ZASSERT(m_physical_device != VK_NULL_HANDLE);
  vkGetPhysicalDeviceProperties(m_physical_device,
                                &m_physical_device_properties);
}

auto VKDevice::init_swapchain(void* window) -> void {
  auto swapchain_builder = vkb::SwapchainBuilder {
    m_physical_device,
    m_device,
    g_surface,
  };

  m_swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  vkb::Swapchain vkb_swapchain =
      swapchain_builder
          .set_desired_format(VkSurfaceFormatKHR {
              .format = m_swapchain_image_format,
              .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(width, height)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  m_swapchain_extent = vkb_swapchain.extent;
  m_swapchain = vkb_swapchain.swapchain;
  m_swapchain_images = vkb_swapchain.get_images().value();
  m_swapchain_image_views = vkb_swapchain.get_image_views().value();
}

auto VKDevice::init_allocator() -> void {
  VmaAllocatorCreateInfo info = {};
  info.physicalDevice = m_physical_device;
  info.device = m_device;
  info.instance = m_instance;
  info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&info, &m_allocator);
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

auto VKDevice::vendor_name() const -> String {
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

auto VKDevice::driver_version() const -> String {
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

VKDevice::~VKDevice() {
  if (not is_initialized()) {
    return;
  }
  vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
  for (int i = 0; i < m_swapchain_image_views.size(); i++) {
    vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
  }

  vkDestroySurfaceKHR(m_instance, g_surface, nullptr);
  vkDestroyDevice(m_device, nullptr);
  vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
  vkDestroyInstance(m_instance, nullptr);
}

} // namespace zod
