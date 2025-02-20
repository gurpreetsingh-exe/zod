#pragma once

#include "./buffer.hh"
#include "./device.hh"
#include "./pipeline.hh"
#include "./shader.hh"
#include "./state.hh"
#include "gpu/backend.hh"

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
  }

  auto create_context(void* glfw_window) -> Shared<GPUContext> override;

  auto create_framebuffer(int, int) -> Shared<GPUFrameBuffer> override {
    TODO();
  }

  auto create_pipeline(PipelineSpec spec) -> Shared<GPUPipeline> override {
    return shared<VKPipeline>(spec);
  }

  auto create_query() -> Shared<GPUQuery> override { TODO(); }

  auto create_shader(GPUShaderCreateInfo info) -> Shared<GPUShader> override {
    return shared<VKShader>(info);
  }

  auto create_state() -> Shared<GPUState> override { return shared<VKState>(); }

  auto create_storage_buffer() -> Shared<GPUStorageBuffer> override {
    return shared<VKStorageBuffer>(1);
  }

  auto create_texture(GPUTextureType, GPUTextureFormat, i32, i32,
                      bool /* bindless */) -> Shared<GPUTexture> override {
    TODO();
  }

  auto create_texture(GPUTextureType, GPUTextureFormat, const fs::path&)
      -> Shared<GPUTexture> override {
    TODO();
  }

  auto create_texture(GPUTextureType, const fs::path&)
      -> Shared<GPUTexture> override {
    TODO();
  }

  auto create_uniform_buffer(usize size_in_bytes)
      -> Shared<GPUUniformBuffer> override {
    return shared<VKUniformBuffer>(size_in_bytes);
  }

  auto create_vertex_buffer() -> Shared<GPUVertexBuffer> override { TODO(); }

private:
  static auto platform_init() -> void;
  static auto platform_init(const VKDevice&) -> void;
};

} // namespace zod
