#include "sodium/widgets/button.hh"
#include "sodium/font.hh"
#include "sodium/style.hh"

namespace zod::sodium {

auto Button::on_mouse_down(const Event& event) -> EventResponse {
  if (not m_enabled) {
    m_hovered = false;
    m_pressed = false;
    return EventResponse::unhandled();
  }

  if (event.button == MouseButton::Left and m_hovered) {
    m_pressed = true;
    return EventResponse::handled().capture_mouse(MouseButton::Left);
  }

  return EventResponse::unhandled();
}

auto Button::on_mouse_up(const Event& event) -> EventResponse {
  if (not m_enabled) {
    m_hovered = false;
    m_pressed = false;
    return EventResponse::unhandled();
  }

  if (event.button == MouseButton::Left) {
    auto was_pressed = m_pressed;
    m_pressed = false;
    if (was_pressed) {
      if (m_hovered and m_on_clicked) {
        m_on_clicked.execute(event);
      }
      return EventResponse::handled();
    }
  }

  return EventResponse::unhandled();
}

auto Button::on_mouse_move(const Event&) -> EventResponse {
  if (not m_enabled) {
    m_hovered = false;
    m_pressed = false;
    return EventResponse::unhandled();
  }

  if (m_hovered) {
    return EventResponse::handled();
  }

  return EventResponse::unhandled();
}

auto Button::compute_desired_size(vec2 available) -> vec2 {
  auto width = Font::get().width(m_name, FontSizeInMenu);
  auto desired =
      max(vec2(width, FontSizeInMenu) + m_style.padding.combined(), m_min_size);
  set_desired_size(desired);
  return desired;
}

auto Button::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }

  rect(m_frame, m_hovered ? m_button_style.hovered_tint : m_style.background)
      .paint(cx);

  auto center = m_frame.position + m_frame.size * 0.5f;
  if (m_icon != IconId::None) {
    auto size = vec2(0.6f) * std::min(m_frame.size.x, m_frame.size.y);
    icon(m_icon, { center - size * 0.5f, size }, { 1, 1, 1, 1 });
  } else {
    auto w = Font::get().width(m_name, FontSizeInMenu);
    Font::get().render_text_center(m_name.c_str(), center.x - w * 0.5, center.y,
                                   FontSizeInMenu);
  }
}

} // namespace zod::sodium
