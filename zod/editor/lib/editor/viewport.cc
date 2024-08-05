#include "viewport.hh"

namespace zod {

Viewport::Viewport() {
  m_framebuffer = GPUBackend::get().create_framebuffer(64, 64);
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture(
      GPUTextureType::Texture2D, 64, 64, false) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->check();
  m_framebuffer->unbind();

  m_shader = GPUBackend::get().create_shader("view_3d");
  m_shader->init_vertex_shader(g_fullscreen);
  m_shader->init_fragment_shader(g_uv);
  m_shader->compile();
}

extern int border;
extern f32 factor;

auto Viewport::draw() -> void {
  f32 b = border * factor;
  GLint view[4];
  glGetIntegerv(GL_VIEWPORT, view);
  // m_framebuffer->bind();
  glViewport(x + b, y + b, w - b * 2, h - b * 2);
  m_shader->bind();
  glDrawArrays(GL_TRIANGLES, 0, 3);
  // m_framebuffer->unbind();
  glViewport(view[0], view[1], view[2], view[3]);
}

auto Viewport::on_event(Event& event) -> void {}

} // namespace zod
