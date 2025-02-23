#pragma once

#include "gpu/batch.hh"
#include "gpu/buffer.hh"
#include "gpu/context.hh"
#include "gpu/framebuffer.hh"
#include "gpu/pipeline.hh"
#include "gpu/query.hh"
#include "gpu/shader.hh"
#include "gpu/state.hh"
#include "gpu/texture.hh"

namespace zod {

class GPUBackend {
protected:
  UniquePtr<ShaderLibrary> m_shader_library = unique<ShaderLibrary>();

public:
  virtual ~GPUBackend() = default;
  static auto get() -> GPUBackend&;
  virtual auto create_batch(const Vector<GPUBufferLayout>&,
                            const Vector<u32>& indices = {})
      -> SharedPtr<GPUBatch> = 0;
  virtual auto create_context(void* /* window */) -> SharedPtr<GPUContext> = 0;
  virtual auto create_framebuffer(int, int) -> SharedPtr<GPUFrameBuffer> = 0;
  virtual auto create_pipeline(PipelineSpec) -> SharedPtr<GPUPipeline> = 0;
  virtual auto create_query() -> SharedPtr<GPUQuery> = 0;
  virtual auto create_shader(GPUShaderCreateInfo) -> SharedPtr<GPUShader> = 0;
  virtual auto create_state() -> SharedPtr<GPUState> = 0;
  virtual auto create_storage_buffer() -> SharedPtr<GPUStorageBuffer> = 0;
  virtual auto create_texture(GPUTextureType, GPUTextureFormat, i32, i32,
                              bool /* bindless */) -> SharedPtr<GPUTexture> = 0;
  virtual auto create_texture(GPUTextureType, GPUTextureFormat, const fs::path&)
      -> SharedPtr<GPUTexture> = 0;
  virtual auto create_texture(GPUTextureType, const fs::path&)
      -> SharedPtr<GPUTexture> = 0;
  virtual auto create_uniform_buffer(usize /* size_in_bytes */)
      -> SharedPtr<GPUUniformBuffer> = 0;
  virtual auto create_vertex_buffer() -> SharedPtr<GPUVertexBuffer> = 0;
  auto get_shader(const String& name) -> SharedPtr<GPUShader> {
    return m_shader_library->get(name);
  }
};

} // namespace zod
