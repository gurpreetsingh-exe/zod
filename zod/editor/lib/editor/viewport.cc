#include "viewport.hh"

namespace zod {

Viewport::Viewport() : m_camera(Camera(64, 64, 90.0f, 0.01f, 100.0f)) {
  m_framebuffer = GPUBackend::get().create_framebuffer(64, 64);
  m_framebuffer->bind();
  GPUAttachment attach = { GPUBackend::get().create_texture(
      GPUTextureType::Texture2D, 64, 64, false) };
  m_framebuffer->add_color_attachment(attach);
  m_framebuffer->add_depth_attachment();
  m_framebuffer->check();
  m_framebuffer->unbind();

  m_shader = GPUBackend::get().create_shader("view_3d");
  m_shader->init_vertex_shader(g_view3d_vert);
  m_shader->init_fragment_shader(g_view3d_frag);
  m_shader->compile();

  f32 position[] = { -1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1,
                     -1, -1, 1,  1, -1, 1,  1, 1, 1,  -1, 1, 1 };
  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, position, 3, 24 },
  };

  m_batch = GPUBackend::get().create_batch(
      format, { 1, 2, 0, 2, 3, 0, 6, 2, 1, 1, 5, 6, 6, 5, 4, 4, 7, 6,
                6, 3, 2, 7, 3, 6, 3, 7, 0, 7, 4, 0, 5, 1, 0, 4, 5, 0 });
}

extern int border;
extern f32 factor;

auto Viewport::draw(DrawData& data) -> void {
  _draw(data, [&] {
    glEnable(GL_DEPTH_TEST);
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    glViewport(x, y, w, h);
    m_shader->bind();
    m_shader->uniform("u_view_projection", m_camera.get_view_projection());
    m_batch->draw(m_shader);
    glDisable(GL_DEPTH_TEST);
    glViewport(view[0], view[1], view[2], view[3]);
  });
}

auto Viewport::on_event(Event& event) -> void {
  if (event.kind == Event::WindowResize) {
    // m_framebuffer->resize(w, h);
    m_camera.resize(w, h);
  }

  m_camera.update(event);
}

} // namespace zod
