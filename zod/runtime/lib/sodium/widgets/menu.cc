#include "sodium/widgets/menu.hh"
#include "sodium/font.hh"
#include "sodium/style.hh"

namespace zod::sodium {

auto Menu::compute_desired_size(vec2 available) -> vec2 {
  auto width = Font::get().width(m_name, FontSizeInMenu);
  auto desired =
      max(vec2(width, FontSizeInMenu) + m_style.padding.combined(), m_min_size);
  set_desired_size(desired);
  if (m_open) {
    auto desired_size = vec2();
    for (auto button : m_buttons) {
      desired_size = max(button->compute_desired_size(available), desired_size);
    }
  }

  return desired;
}

auto Menu::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }
  rect(m_frame, m_style.background).paint(cx);
  auto w = Font::get().width(m_name, FontSizeInMenu);
  auto center = m_frame.position + m_frame.size * 0.5f;
  Font::get().render_text_center(m_name.c_str(), center.x - w * 0.5, center.y,
                                 FontSizeInMenu);
}

auto Menu::arrange(const Rect& bounds) -> void { m_frame = bounds; }

auto Menu::get_children() const -> WidgetChildren {
  auto children = WidgetChildren {};
  children.reserve(m_buttons.size());
  for (const auto& button : m_buttons) {
    if (button) {
      children.push_back(button);
    }
  }
  return children;
}

} // namespace zod::sodium
