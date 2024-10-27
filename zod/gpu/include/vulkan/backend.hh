#pragma once

#include "../backend.hh"
#include "vulkan/device.hh"
#include "vulkan/pipeline.hh"
#include "vulkan/shader.hh"
#include "vulkan/state.hh"

namespace zod {

class VKBackend : public GPUBackend {
  friend class VKDevice;
  static std::string version;

public:
  VKDevice device;

public:
  VKBackend() { VKBackend::platform_init(); }

  static auto get() -> VKBackend& {
    return static_cast<VKBackend&>(GPUBackend::get());
  }

  auto create_batch(const std::vector<GPUBufferLayout>&,
                    const std::vector<u32>& indices = {})
      -> Shared<GPUBatch> override {
    TODO();
    UNREACHABLE();
  }

  auto create_context(void* glfw_window) -> Shared<GPUContext> override;

  auto create_framebuffer(int, int) -> Shared<GPUFrameBuffer> override {
    TODO();
    UNREACHABLE();
  }

  auto create_pipeline(PipelineSpec spec) -> Shared<GPUPipeline> override {
    return shared<VKPipeline>(spec);
  }

  auto create_query() -> Shared<GPUQuery> override {
    TODO();
    UNREACHABLE();
  }

  auto create_shader(std::string name) -> Shared<GPUShader> override {
    return shared<VKShader>(std::move(name));
  }

  auto create_state() -> Shared<GPUState> override { return shared<VKState>(); }

  auto create_storage_buffer() -> Shared<GPUStorageBuffer> override {
    TODO();
    UNREACHABLE();
  }

  auto create_texture(GPUTextureType, GPUTextureFormat, i32, i32,
                      bool /* bindless */) -> Shared<GPUTexture> override {
    TODO();
    UNREACHABLE();
  }

  auto create_texture(GPUTextureType, GPUTextureFormat, const fs::path&)
      -> Shared<GPUTexture> override {
    TODO();
    UNREACHABLE();
  }

  auto create_uniform_buffer(usize /* size_in_bytes */)
      -> Shared<GPUUniformBuffer> override {
    TODO();
    UNREACHABLE();
  }

  auto create_vertex_buffer() -> Shared<GPUVertexBuffer> override {
    TODO();
    UNREACHABLE();
  }

private:
  static auto platform_init() -> void;
  static auto platform_init(const VKDevice&) -> void;
};

} // namespace zod
