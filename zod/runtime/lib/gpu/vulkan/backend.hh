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
  static String version;

public:
  VKDevice device;

public:
  VKBackend() { VKBackend::platform_init(); }

  static auto get() -> VKBackend& {
    return static_cast<VKBackend&>(GPUBackend::get());
  }

  auto create_batch(const Vector<GPUBufferLayout>&,
                    const Vector<u32>& indices = {})
      -> SharedPtr<GPUBatch> override {
    TODO();
  }

  auto create_context(void* glfw_window) -> SharedPtr<GPUContext> override;

  auto create_framebuffer(int, int) -> SharedPtr<GPUFrameBuffer> override {
    TODO();
  }

  auto create_pipeline(GPUPipelineCreateInfo info)
      -> SharedPtr<GPUPipeline> override {
    return shared<VKPipeline>(info);
  }

  auto create_query() -> SharedPtr<GPUQuery> override { TODO(); }

  auto create_shader(GPUShaderCreateInfo info)
      -> SharedPtr<GPUShader> override {
    return shared<VKShader>(info);
  }

  auto create_state() -> SharedPtr<GPUState> override {
    return shared<VKState>();
  }

  auto create_storage_buffer() -> SharedPtr<GPUStorageBuffer> override {
    return shared<VKStorageBuffer>(1);
  }

  auto create_texture(GPUTextureCreateInfo) -> SharedPtr<GPUTexture> override {
    TODO();
  }

  auto create_uniform_buffer(usize size_in_bytes)
      -> SharedPtr<GPUUniformBuffer> override {
    return shared<VKUniformBuffer>(size_in_bytes);
  }

  auto create_vertex_buffer() -> SharedPtr<GPUVertexBuffer> override { TODO(); }

private:
  static auto platform_init() -> void;
  static auto platform_init(const VKDevice&) -> void;
};

} // namespace zod
