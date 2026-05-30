#pragma once

#include "sodium/attribute.hh"
#include "sodium/style.hh"
#include "sodium/widgets/widget.hh"

namespace zod::sodium {

enum class TextJustify {
  Left,
  Center,
  Right,
};

class Text : public Widget {
public:
  Text() = default;
  explicit Text(String text) : m_text(std::move(text)) {}

  auto text() const -> String { return m_text.get(); }
  auto set_text(String text) -> void {
    m_text.set(std::move(text));
    invalidate_layout();
  }
  auto set_text(Attribute<String> text) -> void {
    m_text = std::move(text);
    invalidate_layout();
  }
  template <class Fn>
    requires std::is_invocable_r_v<String, Fn>
  auto set_text(Fn callback) -> void {
    m_text.bind(std::move(callback));
    invalidate_layout();
  }
  template <class ObjectT>
  auto set_text(ObjectT* object, String (ObjectT::*method)()) -> void {
    m_text.bind(object, method);
    invalidate_layout();
  }
  template <class ObjectT>
  auto set_text(const ObjectT* object,
                String (ObjectT::*method)() const) -> void {
    m_text.bind(object, method);
    invalidate_layout();
  }

  auto font_size() const -> f32 { return m_font_size; }
  auto set_font_size(f32 font_size) -> void {
    m_font_size = font_size;
    invalidate_layout();
  }

  auto justification() const -> TextJustify { return m_justification; }
  auto set_justification(TextJustify justification) -> void {
    m_justification = justification;
  }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx& cx) const -> void override;

private:
  auto text_x(const Rect&, f32, TextJustify) const -> f32;

private:
  Attribute<String> m_text;
  f32 m_font_size = FontSizeInMenu;
  TextJustify m_justification = TextJustify::Left;
};

} // namespace zod::sodium
