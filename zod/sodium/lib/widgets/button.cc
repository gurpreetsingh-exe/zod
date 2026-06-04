#include "widgets/button.hh"

namespace zod {

auto SButton::on_event(Event&) -> void {}

auto SButton::draw_imp(Geometry& g) -> void {
  // auto rect = Rect(m_position, m_size, { 0.1f, 0.1f, 0.1f, 1.0f });
  auto position = m_position + 1.0f;
  auto size = m_size - 2.0f;
  auto rect = Rect(position, size, { 0.1f, 0.1f, 0.1f, 1.0f });
  rect.draw(g);
}

auto SButton::compute_desired_size() -> void {
  static constexpr auto font_size = 20.0f;
  m_size = vec2(name.size() * font_size * 0.5f, font_size * 1.2f);
}

} // namespace zod
