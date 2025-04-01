#pragma once

namespace zod {

enum class GPUBackendType {
  OpenGL,
  Vulkan,
  None,
};

enum class GPUDeviceType {
  Nvidia,
  Intel,
  Apple,
  None,
};

auto gpu_platform_string() -> String;

} // namespace zod
