#pragma once

#include "gpu/platform.hh"

namespace zod {

class GPUPlatform {
public:
  bool initialized = false;
  String vendor;
  String version;
  String renderer;
  GPUBackendType backend = GPUBackendType::None;
  GPUDeviceType device = GPUDeviceType::None;

  auto init(GPUBackendType, GPUDeviceType, String /* vendor */,
            String /* version */, String /* renderer */) -> void;
  auto to_string() const -> String;
};

extern GPUPlatform g_platform;

} // namespace zod
