#pragma once

#include <glad/glad.h>

#include "backend.hh"

namespace zod {

class SliceGenerator {
public:
  SliceGenerator(const char* shader, usize size = 192)
      : m_framebuffer(GPUBackend::get().create_framebuffer(size, size)),
        m_shader(GPUBackend::get().create_shader("9-segment")) {
    GPUAttachment attach = { GPUBackend::get().create_texture(
        GPUTextureType::Texture2D, size, size, false) };
    m_framebuffer->add_color_attachment(attach);
    m_framebuffer->check();

    m_shader->init_vertex_shader(g_fullscreen);
    m_shader->init_fragment_shader(shader);
    m_shader->compile();

    // glBindVertexArray(0);
    m_framebuffer->bind();
    m_shader->bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    m_framebuffer->unbind();
  }

  auto get_texture() -> SharedPtr<GPUTexture> {
    return m_framebuffer->get_slot(0);
  }

private:
  SharedPtr<GPUFrameBuffer> m_framebuffer;
  SharedPtr<GPUShader> m_shader;
};

} // namespace zod
