#pragma once

#include "core/shapes.hh"
#include "sodium/widgets/box.hh"
#include "sodium/widgets/button.hh"
#include "sodium/widgets/container.hh"
#include "sodium/widgets/image.hh"
#include "sodium/widgets/overlay.hh"

namespace zod::sodium {

template <class WidgetT>
class WidgetBuilder {
public:
  template <class... Args>
  explicit WidgetBuilder(Args&&... args)
      : m_widget(shared<WidgetT>(std::forward<Args>(args)...)) {}

  auto name(String name) -> WidgetBuilder& {
    m_widget->set_name(std::move(name));
    return *this;
  }

  auto padding(Padding padding) -> WidgetBuilder& {
    m_widget->style().padding = padding;
    return *this;
  }

  auto padding(f32 value) -> WidgetBuilder& {
    m_widget->style().padding = Padding { value };
    return *this;
  }

  auto gap(f32 gap) -> WidgetBuilder& {
    m_widget->style().gap = gap;
    return *this;
  }

  auto background(vec4 color) -> WidgetBuilder& {
    m_widget->style().background = color;
    return *this;
  }

  auto border(vec4 color, f32 thickness = 1.0f) -> WidgetBuilder& {
    m_widget->style().border_color = color;
    m_widget->style().border_thickness = thickness;
    return *this;
  }

  auto desired_size(vec2 size) -> WidgetBuilder& {
    m_widget->set_desired_size(size);
    return *this;
  }

  auto min_size(vec2 size) -> WidgetBuilder& {
    m_widget->set_min_size(size);
    return *this;
  }

  auto hit_test_margin(Padding padding) -> WidgetBuilder& {
    m_widget->set_hit_test_margin(padding);
    return *this;
  }

  auto hit_test_layer(i32 layer) -> WidgetBuilder& {
    m_widget->set_hit_test_layer(layer);
    return *this;
  }

  auto hit_test_priority(i32 priority) -> WidgetBuilder& {
    m_widget->set_hit_test_priority(priority);
    return *this;
  }

  auto cursor(cursor_shape_t cursor) -> WidgetBuilder& {
    m_widget->set_cursor(cursor);
    return *this;
  }

  auto width_override(f32 width) -> WidgetBuilder&
    requires std::is_base_of_v<Box, WidgetT>
  {
    m_widget->set_width_override(width);
    return *this;
  }

  auto height_override(f32 height) -> WidgetBuilder&
    requires std::is_base_of_v<Box, WidgetT>
  {
    m_widget->set_height_override(height);
    return *this;
  }

  auto min_desired_width(f32 width) -> WidgetBuilder&
    requires std::is_base_of_v<Box, WidgetT>
  {
    m_widget->set_min_desired_width(width);
    return *this;
  }

  auto min_desired_height(f32 height) -> WidgetBuilder&
    requires std::is_base_of_v<Box, WidgetT>
  {
    m_widget->set_min_desired_height(height);
    return *this;
  }

  auto max_desired_width(f32 width) -> WidgetBuilder&
    requires std::is_base_of_v<Box, WidgetT>
  {
    m_widget->set_max_desired_width(width);
    return *this;
  }

  auto max_desired_height(f32 height) -> WidgetBuilder&
    requires std::is_base_of_v<Box, WidgetT>
  {
    m_widget->set_max_desired_height(height);
    return *this;
  }

  auto content_halign(Align align) -> WidgetBuilder&
    requires std::is_base_of_v<Box, WidgetT>
  {
    m_widget->set_content_halign(align);
    return *this;
  }

  auto content_valign(Align align) -> WidgetBuilder&
    requires std::is_base_of_v<Box, WidgetT>
  {
    m_widget->set_content_valign(align);
    return *this;
  }

  auto image(vec4 color) -> WidgetBuilder&
    requires std::is_base_of_v<Image, WidgetT>
  {
    m_widget->set_image(color);
    return *this;
  }

  auto accessible_text(String text) -> WidgetBuilder&
    requires std::is_base_of_v<Image, WidgetT>
  {
    m_widget->set_accessible_text(std::move(text));
    return *this;
  }

  auto visibility(Visibility visibility) -> WidgetBuilder& {
    m_widget->set_visibility(visibility);
    return *this;
  }

#define SODIUM_BUILDER_EVENT(Name)                                             \
  WidgetBuilder& Name(EventHandler callback) {                                 \
    m_widget->set_##Name(callback);                                            \
    return *this;                                                              \
  }                                                                            \
  template <class Fn>                                                          \
  WidgetBuilder& Name(Fn&& callback) {                                         \
    return Name(EventHandler(std::forward<Fn>(callback)));                     \
  }                                                                            \
  template <class ObjectT>                                                     \
  WidgetBuilder& Name(ObjectT* object,                                         \
                      EventResponse (ObjectT::*method)(const Event&)) {        \
    m_widget->set_##Name(object, method);                                      \
    return *this;                                                              \
  }                                                                            \
  template <class ObjectT>                                                     \
  WidgetBuilder& Name(const ObjectT* object,                                   \
                      EventResponse (ObjectT::*method)(const Event&) const) {  \
    m_widget->set_##Name(object, method);                                      \
    return *this;                                                              \
  }

  SODIUM_BUILDER_EVENT(on_mouse_down)
  SODIUM_BUILDER_EVENT(on_mouse_up)
  SODIUM_BUILDER_EVENT(on_mouse_move)
  SODIUM_BUILDER_EVENT(on_mouse_enter)
  SODIUM_BUILDER_EVENT(on_mouse_leave)
  SODIUM_BUILDER_EVENT(on_drag_detected)

#undef SODIUM_BUILDER_EVENT

  auto is_focusable(bool focusable) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_focusable(focusable);
    return *this;
  }

  auto is_enabled(bool enabled) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_enabled(enabled);
    return *this;
  }

  auto content_padding(Padding padding) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_content_padding(padding);
    return *this;
  }

  auto content_padding(f32 value) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_content_padding(Padding { value });
    return *this;
  }

  auto button_style(const Button::Style& style) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_button_style(style);
    return *this;
  }

#define SODIUM_BUILDER_BUTTON_EVENT(Name)                                      \
  WidgetBuilder& Name(EventHandler callback)                                   \
    requires std::is_base_of_v<Button, WidgetT>                                \
  {                                                                            \
    m_widget->set_##Name(callback);                                            \
    return *this;                                                              \
  }                                                                            \
  template <class ObjectT>                                                     \
  WidgetBuilder& Name(ObjectT* object,                                         \
                      EventResponse (ObjectT::*method)(const Event&))          \
    requires std::is_base_of_v<Button, WidgetT>                                \
  {                                                                            \
    m_widget->set_##Name(object, method);                                      \
    return *this;                                                              \
  }                                                                            \
  template <class ObjectT>                                                     \
  WidgetBuilder& Name(const ObjectT* object,                                   \
                      EventResponse (ObjectT::*method)(const Event&) const)    \
    requires std::is_base_of_v<Button, WidgetT>                                \
  {                                                                            \
    m_widget->set_##Name(object, method);                                      \
    return *this;                                                              \
  }

  SODIUM_BUILDER_BUTTON_EVENT(on_clicked)

#undef SODIUM_BUILDER_BUTTON_EVENT

  auto slot(SharedPtr<Widget> child, const SlotStyle& style = {})
      -> WidgetBuilder&
    requires std::is_base_of_v<Container, WidgetT>
  {
    if constexpr (std::is_base_of_v<Overlay, WidgetT>) {
      m_widget->add_overlay_child(child, style);
    } else {
      m_widget->add_child(child, style);
    }
    return *this;
  }

  template <class ChildT>
  auto slot(WidgetBuilder<ChildT>&& child, const SlotStyle& style = {})
      -> WidgetBuilder&
    requires std::is_base_of_v<Container, WidgetT>
  {
    return slot(child.build(), style);
  }

  auto child(SharedPtr<Widget> child, const SlotStyle& style = {})
      -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    m_widget->set_child(std::move(child), style);
    return *this;
  }

  template <class ChildT>
  auto child(WidgetBuilder<ChildT>&& child, const SlotStyle& style = {})
      -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    return child(child.build(), style);
  }

  auto operator+(SlotContent content) -> WidgetBuilder&
    requires std::is_base_of_v<Container, WidgetT>
  {
    if constexpr (std::is_base_of_v<Overlay, WidgetT>) {
      m_widget->add_overlay_child(std::move(content.child), content.style,
                                  content.layer);
    } else {
      m_widget->add_child(std::move(content.child), content.style);
    }
    return *this;
  }

  auto operator[](SharedPtr<Widget> child) -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    m_widget->set_child(std::move(child));
    return *this;
  }

  template <class ChildT>
  auto operator[](WidgetBuilder<ChildT>&& child) -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    return (*this)[child.build()];
  }

  template <class ChildT>
  auto operator[](WidgetBuilder<ChildT>& child) -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    return (*this)[child.build()];
  }

  auto build() -> SharedPtr<WidgetT> { return m_widget; }
  auto operator*() -> SharedPtr<WidgetT> { return build(); }
  operator SharedPtr<WidgetT>() { return build(); }

private:
  SharedPtr<WidgetT> m_widget;
};

template <class WidgetT, class... Args>
auto create(Args&&... args) -> WidgetBuilder<WidgetT> {
  return WidgetBuilder<WidgetT>(std::forward<Args>(args)...);
}

} // namespace zod::sodium
