#pragma once

#include "sodium/icon.hh"
#include "sodium/widgets/compound_widget.hh"

namespace zod::sodium {

class Button : public CompoundWidget {
public:
  struct Style {
    vec4 hovered_tint;
  };

  Button() = default;
  Button(IconId icon) : m_icon(icon) {}

  auto set_focusable(bool focusable) -> void { m_focusable = focusable; }
  auto set_enabled(bool enabled) -> void { m_enabled = enabled; }
  auto set_button_style(Style style) -> void { m_button_style = style; }

#define SODIUM_BUTTON_EVENT_SETTER(Name, Field)                                \
  void set_##Name(EventHandler callback) { Field = callback; }                 \
  template <class ObjectT>                                                     \
  void set_##Name(ObjectT* object,                                             \
                  EventResponse (ObjectT::*method)(const Event&)) {            \
    Field = EventHandler(object, method);                                      \
  }                                                                            \
  template <class ObjectT>                                                     \
  void set_##Name(const ObjectT* object,                                       \
                  EventResponse (ObjectT::*method)(const Event&) const) {      \
    Field = EventHandler(object, method);                                      \
  }

  SODIUM_BUTTON_EVENT_SETTER(on_clicked, m_on_clicked)

#undef SODIUM_BUTTON_EVENT_SETTER

  auto set_content_padding(Padding padding) -> void {
    m_style.padding = padding;
  }

  auto on_mouse_down(const Event&) -> EventResponse override;
  auto on_mouse_up(const Event&) -> EventResponse override;
  auto on_mouse_move(const Event&) -> EventResponse override;
  auto compute_desired_size(vec2) -> vec2 override;
  auto paint(PaintCx&) const -> void override;

private:
  bool m_focusable = true;
  bool m_enabled = true;
  bool m_pressed = false;
  IconId m_icon = IconId::None;
  Style m_button_style = {};
  EventHandler m_on_clicked = {};
};

} // namespace zod::sodium
