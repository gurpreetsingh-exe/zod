#include "outliner.hh"

namespace zod {

Outliner::Outliner() {
  m_shader = GPUBackend::get().create_shader("outliner");
  m_shader->init_vertex_shader(g_rect_vert);
  m_shader->init_fragment_shader(g_outliner_frag);
  m_shader->compile();
}

extern int border;
extern f32 factor;

auto Outliner::draw(DrawData& data) -> void {
  _draw(data, [&] {
    m_shader->bind();
    m_shader->uniform("u_x", 0);
    m_shader->uniform("u_y", 0);
    m_shader->uniform("u_width", w);
    m_shader->uniform("u_height", h);
    m_shader->uniform("u_border", border);
    m_shader->uniform("u_padding", padding);
    // fmt::println("{} {}", w, h);
    m_shader->uniform("u_color", glm::vec3(0.16f, 0.17f, 0.18f));
    data.batch->draw(m_shader);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
  });
}

auto Outliner::on_event(Event& event) -> void {}

} // namespace zod
