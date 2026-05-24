#pragma once

#include "sodium/widgets/compound_widget.hh"
#include "sodium/icon.hh"

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
  auto set_on_clicked(EventHandler callback) -> void {
    m_on_clicked = callback;
  }

  template <class ObjectT>
  auto set_on_clicked(ObjectT* object,
                      EventResponse (ObjectT::*method)(const Event&)) -> void {
    m_on_clicked =
        EventHandler([object, method](const Event& event) -> EventResponse {
          return (object->*method)(event);
        });
  }

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
