#include "widgets/button.hh"
#include "font.hh"

namespace zod {

auto SButton::on_event(Event&) -> void {}

auto SButton::draw_imp(Geometry& g) -> void {
  // auto rect = Rect(m_position, m_size, { 0.1f, 0.1f, 0.1f, 1.0f });
  auto position = m_position + 1.0f;
  auto size = m_size - 2.0f;
  auto rect = Rect(position, size, { 0.1f, 0.1f, 0.1f, 1.0f });
  rect.draw(g);
  Font::get().render_text_center(name.c_str(), rect.top_left().x + 8.0f,
                                 position.y + size.y * 0.5, 0.6);
}

auto SButton::compute_desired_size() -> void {
  m_size = vec2(name.size() * Font::size * 0.5, Font::size * 1.2);
}

} // namespace zod
