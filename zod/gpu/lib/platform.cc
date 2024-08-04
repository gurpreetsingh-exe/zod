#include "platform_private.hh"

namespace zod {

GPUPlatform g_platform;

auto GPUPlatform::init(GPUBackendType backend, GPUDeviceType device,
                       std::string vendor, std::string version,
                       std::string renderer) -> void {
  this->backend = backend;
  this->device = device;
  this->vendor = std::move(vendor);
  this->version = std::move(version);
  this->renderer = std::move(renderer);
}

} // namespace zod
