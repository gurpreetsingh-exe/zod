#include "vulkan/backend.hh"
#include "vulkan/context.hh"

#include "platform_private.hh"

namespace zod {

auto VKBackend::platform_init() -> void {
  g_platform.init(GPUBackendType::Vulkan, GPUDeviceType::None, "", "", "");
}

auto VKBackend::platform_init(const VKDevice& device) -> void {
  g_platform.init(GPUBackendType::Vulkan, GPUDeviceType::None, "", "", "");
}

auto VKBackend::create_context(void* glfw_window) -> Shared<GPUContext> {
  if (not device.is_initialized()) {
    device.init(glfw_window);
  }

  return shared<VKContext>(glfw_window);
}

} // namespace zod
