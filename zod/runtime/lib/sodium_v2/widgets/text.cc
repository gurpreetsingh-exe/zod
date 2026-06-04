#include "sodium_v2/widgets/text.hh"
#include "sodium_v2/font.hh"

namespace zod::sodium {

auto Text::compute_desired_size(vec2) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }

  auto text = m_text.get();
  auto font_size = m_font_size;
  auto size = vec2(Font::get().width(text, font_size), font_size) +
              m_style.padding.combined();
  return cache_desired_size(max(size, m_min_size));
}

auto Text::arrange(const Rect& bounds) -> void { m_frame = bounds; }

auto Text::text_x(const Rect& inner, f32 text_width,
                  TextJustify justification) const -> f32 {
  switch (justification) {
    case TextJustify::Center:
      return inner.position.x + (inner.size.x - text_width) * 0.5f;
    case TextJustify::Right:
      return inner.position.x + inner.size.x - text_width;
    case TextJustify::Left:
      break;
  }

  return inner.position.x;
}

auto Text::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }

  push_self_draws(cx);
  auto text = m_text.get();
  auto font_size = m_font_size;
  auto inner = m_frame.padding(m_style.padding);
  auto text_width = Font::get().width(text, font_size);
  auto baseline_y = inner.position.y + inner.size.y * 0.5f + font_size * 0.4f;
  Font::get().render_text(cx, text.c_str(),
                          text_x(inner, text_width, m_justification),
                          baseline_y, font_size, font_size);
}

} // namespace zod::sodium
