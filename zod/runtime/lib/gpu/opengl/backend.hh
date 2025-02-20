#pragma once

#include "./batch.hh"
#include "./buffer.hh"
#include "./context.hh"
#include "./framebuffer.hh"
#include "./pipeline.hh"
#include "./query.hh"
#include "./shader.hh"
#include "./state.hh"
#include "gpu/backend.hh"

namespace zod {

class GLBackend : public GPUBackend {
  static std::string version;

public:
  GLBackend() { GLBackend::platform_init(); }

  auto create_batch(const std::vector<GPUBufferLayout>& layouts,
                    const std::vector<u32>& indices)
      -> Shared<GPUBatch> override {
    return shared<GLBatch>(layouts, indices);
  }

  auto create_context(void* glfw_window) -> Shared<GPUContext> override {
    return shared<GLContext>(glfw_window);
  }

  auto create_framebuffer(int w, int h) -> Shared<GPUFrameBuffer> override {
    return shared<GLFrameBuffer>(w, h);
  }

  auto create_pipeline(PipelineSpec spec) -> Shared<GPUPipeline> override {
    return shared<GLPipeline>(spec);
  }

  auto create_query() -> Shared<GPUQuery> override { return shared<GLQuery>(); }

  auto create_shader(GPUShaderCreateInfo info) -> Shared<GPUShader> override {
    Shared<GPUShader> shader = shared<GLShader>(info);
    m_shader_library->add(shader->name, shader);
    return shader;
  }

  auto create_state() -> Shared<GPUState> override { return shared<GLState>(); }

  auto create_storage_buffer() -> Shared<GPUStorageBuffer> override {
    return shared<GLStorageBuffer>();
  }

  auto create_texture(GPUTextureType type, GPUTextureFormat format, i32 width,
                      i32 height, bool bindless)
      -> Shared<GPUTexture> override {
    return shared<GLTexture>(type, format, width, height, bindless);
  }

  auto create_texture(GPUTextureType type, GPUTextureFormat format,
                      const fs::path& path) -> Shared<GPUTexture> override {
    return shared<GLTexture>(type, format, path);
  }

  auto create_texture(GPUTextureType type, const fs::path& path)
      -> Shared<GPUTexture> override {
    return shared<GLTexture>(type, path);
  }

  auto create_uniform_buffer(usize size) -> Shared<GPUUniformBuffer> override {
    return shared<GLUniformBuffer>(size);
  }

  auto create_vertex_buffer() -> Shared<GPUVertexBuffer> override {
    return shared<GLVertexBuffer>();
  }

private:
  static auto platform_init() -> void;
};

} // namespace zod
