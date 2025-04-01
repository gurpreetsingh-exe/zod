#include "platform_private.hh"

namespace zod {

GPUPlatform g_platform;

auto GPUPlatform::init(GPUBackendType backend, GPUDeviceType device,
                       String vendor, String version, String renderer) -> void {
  this->backend = backend;
  this->device = device;
  this->vendor = std::move(vendor);
  this->version = std::move(version);
  this->renderer = std::move(renderer);
}

auto GPUPlatform::to_string() const -> String {
  return fmt::format("{} {} {}", vendor, version, renderer);
}

auto gpu_platform_string() -> String { return g_platform.to_string(); }

} // namespace zod
