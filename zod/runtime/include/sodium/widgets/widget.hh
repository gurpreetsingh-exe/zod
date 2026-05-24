#pragma once

#include "core/shapes.hh"
#include "sodium/cursor_reply.hh"
#include "sodium/layout.hh"
#include "sodium/metadata.hh"
#include "sodium/paint.hh"

namespace zod::sodium {

class Widget;
class WidgetPath {
public:
  auto size() const -> usize { return m_widgets.size(); }
  auto empty() const -> bool { return m_widgets.empty(); }
  auto clear() -> void { m_widgets.clear(); }
  auto resize(usize size) -> void { m_widgets.resize(size); }
  auto push(Widget* widget) -> void { m_widgets.push_back(widget); }
  auto contains(const Widget* widget) const -> bool;
  auto operator[](usize index) const -> Widget* { return m_widgets[index]; }
  auto begin() const { return m_widgets.begin(); }
  auto end() const { return m_widgets.end(); }

private:
  Vector<Widget*> m_widgets = {};
};

struct WidgetStyle {
  Padding padding {};
  f32 gap = 0.0f;
  vec4 background { 0.0f, 0.0f, 0.0f, 0.0f };
  vec4 border_color { 0.0f, 0.0f, 0.0f, 0.0f };
  f32 border_thickness = 0.0f;
};

using WidgetChildren = Vector<SharedPtr<Widget>>;

class Widget {
public:
  virtual ~Widget() = default;

  auto name() const -> const String& { return m_name; }
  auto set_name(String name) -> void { m_name = std::move(name); }

  auto frame() const -> const Rect& { return m_frame; }
  auto desired_size() const -> const vec2& { return m_desired_size; }
  auto style() const -> const WidgetStyle& { return m_style; }
  auto style() -> WidgetStyle& { return m_style; }
  auto visibility() const -> Visibility { return m_visibility; }
  auto hovered() const -> bool { return m_hovered; }
  auto set_visibility(Visibility visibility) -> void {
    m_visibility = visibility;
  }
  auto set_min_size(vec2 size) -> void { m_min_size = size; }
  auto set_hit_test_margin(Padding padding) -> void {
    m_hit_test_margin = padding;
  }
  auto set_hit_test_layer(i32 layer) -> void { m_hit_test_layer = layer; }
  auto set_hit_test_priority(i32 priority) -> void {
    m_hit_test_priority = priority;
  }
  auto set_cursor(cursor_shape_t cursor) -> void {
    m_has_cursor = true;
    m_cursor = cursor;
  }
  auto clear_cursor() -> void { m_has_cursor = false; }
  auto hit_test_layer() const -> i32 { return m_hit_test_layer; }
  auto hit_test_priority() const -> i32 { return m_hit_test_priority; }
  auto hit_test_bounds() const -> Rect {
    return m_frame.margin(m_hit_test_margin);
  }

  auto add_metadata(SharedPtr<IWidgetMetaData> metadata) -> void {
    m_metadata.push_back(std::move(metadata));
  }

  template <class MetaDataT>
  auto get_metadata() const -> SharedPtr<MetaDataT> {
    for (const auto& metadata : m_metadata) {
      if (metadata and metadata->is_of_type<MetaDataT>()) {
        return std::static_pointer_cast<MetaDataT>(metadata);
      }
    }
    return nullptr;
  }

  template <class MetaDataT>
  auto find_or_add_metadata() -> SharedPtr<MetaDataT> {
    if (auto metadata = get_metadata<MetaDataT>()) {
      return metadata;
    }

    auto metadata = shared<MetaDataT>();
    add_metadata(metadata);
    return metadata;
  }

#define SODIUM_WIDGET_EVENT_SETTER(Name, Field)                                \
  void set_##Name(EventHandler callback) {                                     \
    find_or_add_metadata<WidgetMouseEventsMetaData>()->Field = callback;       \
  }                                                                            \
  template <class ObjectT>                                                     \
  void set_##Name(ObjectT* object,                                             \
                  EventResponse (ObjectT::*method)(const Event&)) {            \
    set_##Name(EventHandler(object, method));                                  \
  }                                                                            \
  template <class ObjectT>                                                     \
  void set_##Name(const ObjectT* object,                                       \
                  EventResponse (ObjectT::*method)(const Event&) const) {      \
    set_##Name(EventHandler(object, method));                                  \
  }

  SODIUM_WIDGET_EVENT_SETTER(on_mouse_down, mouse_down)
  SODIUM_WIDGET_EVENT_SETTER(on_mouse_up, mouse_up)
  SODIUM_WIDGET_EVENT_SETTER(on_mouse_move, mouse_move)
  SODIUM_WIDGET_EVENT_SETTER(on_mouse_enter, mouse_enter)
  SODIUM_WIDGET_EVENT_SETTER(on_mouse_leave, mouse_leave)
  SODIUM_WIDGET_EVENT_SETTER(on_drag_detected, drag_detected)

#undef SODIUM_WIDGET_EVENT_SETTER

  virtual auto event(const Event&) -> EventResponse;
  virtual auto on_mouse_down(const Event&) -> EventResponse;
  virtual auto on_mouse_up(const Event&) -> EventResponse;
  virtual auto on_mouse_move(const Event&) -> EventResponse;
  virtual auto on_mouse_enter(const Event&) -> void;
  virtual auto on_mouse_leave(const Event&) -> void;
  virtual auto on_drag_detected(const Event&) -> EventResponse;
  virtual auto on_cursor_query(const Event&) const -> CursorReply;
  virtual auto on_key_down(const Event&) -> EventResponse {
    return EventResponse::unhandled();
  }
  virtual auto on_key_up(const Event&) -> EventResponse {
    return EventResponse::unhandled();
  }
  virtual auto on_key_repeat(const Event&) -> EventResponse {
    return EventResponse::unhandled();
  }
  virtual auto on_window_resize(const Event&) -> EventResponse {
    return EventResponse::unhandled();
  }
  virtual auto on_window_close(const Event&) -> EventResponse {
    return EventResponse::unhandled();
  }
  virtual auto set_desired_size(vec2 size) -> void { m_desired_size = size; }
  virtual auto compute_desired_size(vec2) -> vec2 = 0;
  virtual auto arrange(const Rect&) -> void = 0;
  virtual auto paint(PaintCx&) const -> void = 0;
  virtual auto get_children() const -> WidgetChildren { return {}; }
  auto invalidate_layout() -> void { m_layout_invalidated = true; }
  auto needs_layout() const -> bool;
  auto clear_layout_invalidated() -> void;

protected:
  auto cache_desired_size(vec2 desired) const -> vec2 {
    m_desired_size = desired;
    return m_desired_size;
  }

  auto apply_event_reply(const Event&, EventResponse) -> EventResponse;
  auto drag_detected_reply(const Event&) -> EventResponse;
  auto update_hover_path(const WidgetPath&, const Event&) -> void;
  auto route_pointer_event(const Event&, const WidgetPath&) -> EventResponse;
  auto update_cursor(const Event&, const WidgetPath&) -> void;
  auto route_tree_event(const Event&) -> EventResponse;
  auto push_self_draws(PaintCx&) const -> void;

  String m_name;
  WidgetStyle m_style {};
  Rect m_frame {};
  Padding m_hit_test_margin = {};
  i32 m_hit_test_layer = 0;
  i32 m_hit_test_priority = 0;
  bool m_has_cursor = false;
  cursor_shape_t m_cursor = cursor_shape_t::Arrow;
  mutable vec2 m_desired_size {};
  mutable vec2 m_min_size {};
  Visibility m_visibility = Visibility::Visible;
  bool m_hovered = false;
  bool m_detecting_drag = false;
  bool m_drag_detected = false;
  MouseButton m_drag_button = MouseButton::None;
  vec2 m_drag_start = {};
  f32 m_drag_threshold = 2.0f;
  WidgetPath m_hovered_path = {};
  Widget* m_mouse_captor = nullptr;
  MouseButton m_mouse_capture_button = MouseButton::None;
  bool m_layout_invalidated = false;
  Vector<SharedPtr<IWidgetMetaData>> m_metadata = {};
};

} // namespace zod::sodium
