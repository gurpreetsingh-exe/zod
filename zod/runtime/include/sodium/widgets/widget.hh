#pragma once

#include "core/shapes.hh"
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

  auto set_on_mouse_down(EventHandler callback) -> void {
    find_or_add_metadata<WidgetMouseEventsMetaData>()->mouse_down = callback;
  }
  auto set_on_mouse_up(EventHandler callback) -> void {
    find_or_add_metadata<WidgetMouseEventsMetaData>()->mouse_up = callback;
  }
  auto set_on_mouse_move(EventHandler callback) -> void {
    find_or_add_metadata<WidgetMouseEventsMetaData>()->mouse_move = callback;
  }
  auto set_on_mouse_enter(EventHandler callback) -> void {
    find_or_add_metadata<WidgetMouseEventsMetaData>()->mouse_enter = callback;
  }
  auto set_on_mouse_leave(EventHandler callback) -> void {
    find_or_add_metadata<WidgetMouseEventsMetaData>()->mouse_leave = callback;
  }
  auto set_on_drag_detected(EventHandler callback) -> void {
    find_or_add_metadata<WidgetMouseEventsMetaData>()->drag_detected = callback;
  }

  virtual auto event(const Event&) -> EventResponse;
  virtual auto on_mouse_down(const Event&) -> EventResponse;
  virtual auto on_mouse_up(const Event&) -> EventResponse;
  virtual auto on_mouse_move(const Event&) -> EventResponse;
  virtual auto on_mouse_enter(const Event&) -> void;
  virtual auto on_mouse_leave(const Event&) -> void;
  virtual auto on_drag_detected(const Event&) -> EventResponse;
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
  auto find_path_at(vec2, WidgetPath&) -> bool;
  auto update_hover_path(const WidgetPath&, const Event&) -> void;
  auto route_pointer_event(const Event&, const WidgetPath&) -> EventResponse;
  auto route_tree_event(const Event&) -> EventResponse;
  auto push_self_draws(PaintCx&) const -> void;

  String m_name;
  WidgetStyle m_style {};
  Rect m_frame {};
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
