#pragma once

#include "../backend.hh"
#include "vulkan/device.hh"

namespace zod {

class VKBackend : public GPUBackend {
  friend class VKDevice;
  static std::string version;

public:
  VKDevice device;

public:
  VKBackend() { VKBackend::platform_init(); }

  auto create_batch(const std::vector<GPUBufferLayout>&,
                    const std::vector<u32>& indices = {}) -> Shared<GPUBatch> {
    TODO();
    UNREACHABLE();
  }

  auto create_context(void* glfw_window) -> Shared<GPUContext> override;

  auto create_framebuffer(int, int) -> Shared<GPUFrameBuffer> {
    TODO();
    UNREACHABLE();
  }
  auto create_query() -> Shared<GPUQuery> {
    TODO();
    UNREACHABLE();
  }
  auto create_renderer() -> Shared<GPURenderer> {
    TODO();
    UNREACHABLE();
  }
  auto create_shader(std::string /* name */) -> Shared<GPUShader> {
    TODO();
    UNREACHABLE();
  }
  auto create_state() -> Shared<GPUState> {
    TODO();
    UNREACHABLE();
  }
  auto create_storage_buffer() -> Shared<GPUStorageBuffer> {
    TODO();
    UNREACHABLE();
  }
  auto create_texture(GPUTextureType, GPUTextureFormat, i32, i32,
                      bool /* bindless */) -> Shared<GPUTexture> {
    TODO();
    UNREACHABLE();
  }
  auto create_texture(GPUTextureType, GPUTextureFormat, const fs::path&)
      -> Shared<GPUTexture> {
    TODO();
    UNREACHABLE();
  }
  auto create_uniform_buffer(usize /* size_in_bytes */)
      -> Shared<GPUUniformBuffer> {
    TODO();
    UNREACHABLE();
  }
  auto create_vertex_buffer() -> Shared<GPUVertexBuffer> {
    TODO();
    UNREACHABLE();
  }

private:
  static auto platform_init() -> void;
  static auto platform_init(const VKDevice&) -> void;
};

} // namespace zod
