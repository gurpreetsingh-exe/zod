#pragma once

#include "../backend.hh"
#include "opengl/batch.hh"
#include "opengl/buffer.hh"
#include "opengl/context.hh"
#include "opengl/framebuffer.hh"
#include "opengl/renderer.hh"
#include "opengl/shader.hh"

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

  auto create_renderer() -> Shared<GPURenderer> override {
    return shared<GLRenderer>();
  }

  auto create_shader(std::string name) -> Shared<GPUShader> override {
    return shared<GLShader>(std::move(name));
  }

  auto create_texture(GPUTextureType type, i32 width, i32 height, bool bindless)
      -> Shared<GPUTexture> override {
    return shared<GLTexture>(type, width, height, bindless);
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
