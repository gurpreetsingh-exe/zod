#pragma once

#include "platform.hh"

namespace zod {

class GPUPlatform {
public:
  bool initialized = false;
  std::string vendor;
  std::string version;
  std::string renderer;
  GPUBackendType backend = GPUBackendType::None;
  GPUDeviceType device = GPUDeviceType::None;

  auto init(GPUBackendType, GPUDeviceType, std::string /* vendor */,
            std::string /* version */, std::string /* renderer */) -> void;
};

extern GPUPlatform g_platform;

} // namespace zod
