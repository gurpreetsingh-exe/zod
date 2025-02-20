#include "./backend.hh"
#include "./context.hh"

#include "../platform_private.hh"

namespace zod {

auto VKBackend::platform_init() -> void {
  g_platform.init(GPUBackendType::Vulkan, GPUDeviceType::None, "", "", "");
}

auto VKBackend::platform_init(const VKDevice& device) -> void {
  const auto& properties = device.get_physical_device_properties();
  auto device_type = device.device_type();
  auto vendor = device.vendor_name();
  auto version = device.driver_version();
  g_platform.init(GPUBackendType::Vulkan, device_type, std::move(vendor),
                  std::move(version), properties.deviceName);
}

auto VKBackend::create_context(void* glfw_window) -> Shared<GPUContext> {
  if (not device.is_initialized()) {
    device.init(glfw_window);
  }

  return shared<VKContext>(glfw_window);
}

} // namespace zod
