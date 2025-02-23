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
  static String version;

public:
  GLBackend() { GLBackend::platform_init(); }

  auto create_batch(const Vector<GPUBufferLayout>& layouts,
                    const Vector<u32>& indices)
      -> SharedPtr<GPUBatch> override {
    return shared<GLBatch>(layouts, indices);
  }

  auto create_context(void* glfw_window) -> SharedPtr<GPUContext> override {
    return shared<GLContext>(glfw_window);
  }

  auto create_framebuffer(int w, int h) -> SharedPtr<GPUFrameBuffer> override {
    return shared<GLFrameBuffer>(w, h);
  }

  auto create_pipeline(PipelineSpec spec) -> SharedPtr<GPUPipeline> override {
    return shared<GLPipeline>(spec);
  }

  auto create_query() -> SharedPtr<GPUQuery> override {
    return shared<GLQuery>();
  }

  auto create_shader(GPUShaderCreateInfo info)
      -> SharedPtr<GPUShader> override {
    SharedPtr<GPUShader> shader = shared<GLShader>(info);
    m_shader_library->add(shader->name, shader);
    return shader;
  }

  auto create_state() -> SharedPtr<GPUState> override {
    return shared<GLState>();
  }

  auto create_storage_buffer() -> SharedPtr<GPUStorageBuffer> override {
    return shared<GLStorageBuffer>();
  }

  auto create_texture(GPUTextureType type, GPUTextureFormat format, i32 width,
                      i32 height, bool bindless)
      -> SharedPtr<GPUTexture> override {
    return shared<GLTexture>(type, format, width, height, bindless);
  }

  auto create_texture(GPUTextureType type, GPUTextureFormat format,
                      const fs::path& path) -> SharedPtr<GPUTexture> override {
    return shared<GLTexture>(type, format, path);
  }

  auto create_texture(GPUTextureType type, const fs::path& path)
      -> SharedPtr<GPUTexture> override {
    return shared<GLTexture>(type, path);
  }

  auto create_uniform_buffer(usize size)
      -> SharedPtr<GPUUniformBuffer> override {
    return shared<GLUniformBuffer>(size);
  }

  auto create_vertex_buffer() -> SharedPtr<GPUVertexBuffer> override {
    return shared<GLVertexBuffer>();
  }

private:
  static auto platform_init() -> void;
};

} // namespace zod
