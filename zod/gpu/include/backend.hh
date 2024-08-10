#pragma once

#include "batch.hh"
#include "buffer.hh"
#include "context.hh"
#include "framebuffer.hh"
#include "renderer.hh"
#include "shader.hh"
#include "texture.hh"

namespace zod {

class GPUBackend {
protected:
  Unique<ShaderLibrary> m_shader_library = unique<ShaderLibrary>();

public:
  virtual ~GPUBackend() = default;
  static auto get() -> GPUBackend&;
  virtual auto create_batch(const std::vector<GPUBufferLayout>&,
                            const std::vector<u32>& indices = {})
      -> Shared<GPUBatch> = 0;
  virtual auto create_context(void* /* window */) -> Shared<GPUContext> = 0;
  virtual auto create_framebuffer(int, int) -> Shared<GPUFrameBuffer> = 0;
  virtual auto create_renderer() -> Shared<GPURenderer> = 0;
  virtual auto create_shader(std::string /* name */) -> Shared<GPUShader> = 0;
  virtual auto create_storage_buffer() -> Shared<GPUStorageBuffer> = 0;
  virtual auto create_texture(GPUTextureType, i32, i32, bool /* bindless */)
      -> Shared<GPUTexture> = 0;
  virtual auto create_uniform_buffer(usize /* size_in_bytes */)
      -> Shared<GPUUniformBuffer> = 0;
  virtual auto create_vertex_buffer() -> Shared<GPUVertexBuffer> = 0;
  auto get_shader(const std::string& name) -> Shared<GPUShader> {
    return m_shader_library->get(name);
  }
};

} // namespace zod
