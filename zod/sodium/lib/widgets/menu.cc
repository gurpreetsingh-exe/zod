#include "widgets/menu.hh"
#include "engine/font.hh"
#include "widgets/rect.hh"

namespace zod {

auto SMenu::on_event(Event&) -> void {}

auto SMenu::draw_imp(Geometry& g) -> void {
  // auto rect = Rect(m_position, m_size, { 0.1f, 0.1f, 0.1f, 1.0f });
  auto rect =
      Rect(m_position - 1.0f, m_size + 2.0f, { 0.2f, 0.2f, 0.2f, 1.0f });
  rect.draw(g);
  for (auto& child : m_children) { child->draw(g); }
}

auto SMenu::compute_desired_size() -> void {
  m_size = {};
  for (usize i = 0; i < m_children.size(); ++i) {
    auto child = m_children[i];
    child->compute_desired_size();
    auto size = child->get_size();
    auto location = vec2(m_position.x, m_position.y + i * size.y);
    m_size.x = std::max(m_size.x, size.x);
    m_size.y += size.y;
    child->set_position(location);
  }

  for (auto& child : m_children) {
    child->set_size(vec2(m_size.x, m_size.y / m_children.size()));
  }
}

} // namespace zod
