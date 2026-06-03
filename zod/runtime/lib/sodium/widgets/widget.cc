#include "sodium/widgets/widget.hh"
#include "application/application.hh"
#include "sodium/hit_test.hh"

namespace zod::sodium {

struct WeakWidgetPath {
  Vector<std::weak_ptr<Widget>> widgets = {};
};

struct PointerEventState {
  WeakWidgetPath hovered_path = {};
  WeakWidgetPath capture_path = {};
  MouseButton capture_button = MouseButton::None;
  Vector<String> debug_hover_path = {};
  String debug_mouse_capture = {};
};

static auto s_next_widget_id = u64(0);
static auto s_pointer_state = PointerEventState();

static auto event_reply(EventResponse widget_reply, EventResponse child_reply,
                        bool handled) -> EventResponse {
  if (widget_reply) {
    return widget_reply;
  }
  if (child_reply) {
    return child_reply;
  }
  if (handled) {
    return EventResponse::handled();
  }
  return EventResponse::unhandled();
}

Widget::Widget() : m_debug_id(++s_next_widget_id) {}

static auto is_pointer_event(const Event& event) -> bool {
  return event.kind == Event::MouseDown or event.kind == Event::MouseUp or
         event.kind == Event::MouseMove;
}

static auto debug_widget_label(const SharedPtr<Widget>& widget, usize index)
    -> String {
  auto name = widget->name().empty() ? String("<unnamed>") : widget->name();
  auto frame = widget->frame();
  return fmt::format("{}: {} {} id={} frame=({}, {}, {}, {})", index, name,
                     typeid(*widget).name(), widget->debug_id(),
                     frame.position.x, frame.position.y, frame.size.x,
                     frame.size.y);
}

static auto debug_path_labels(const WidgetPath& path) -> Vector<String> {
  auto labels = Vector<String>();
  for (auto i = usize(0); i < path.size(); ++i) {
    labels.push_back(debug_widget_label(path[i], i));
  }
  return labels;
}

static auto hover_visible_path(const WidgetPath& path) -> WidgetPath {
  auto visible = WidgetPath();
  for (const auto& widget : path) {
    if (not widget->hover_path_visible()) {
      break;
    }
    visible.push(widget);
  }
  return visible;
}

static auto make_weak_path(const WidgetPath& path) -> WeakWidgetPath {
  auto weak = WeakWidgetPath();
  for (const auto& widget : path) { weak.widgets.push_back(widget); }
  return weak;
}

static auto path_down_to(const WidgetPath& path,
                         const SharedPtr<Widget>& target) -> WidgetPath {
  auto result = WidgetPath();
  for (const auto& widget : path) {
    result.push(widget);
    if (widget == target) {
      return result;
    }
  }
  return {};
}

static auto find_child(const SharedPtr<Widget>& parent,
                       const SharedPtr<Widget>& widget) -> SharedPtr<Widget> {
  for (auto child : parent->get_children()) {
    if (child == widget) {
      return child;
    }
  }
  return nullptr;
}

static auto resolve_weak_path(const SharedPtr<Widget>& root,
                              const WeakWidgetPath& weak, bool truncate)
    -> WidgetPath {
  auto path = WidgetPath();
  if (not root or weak.widgets.empty()) {
    return path;
  }

  auto current = weak.widgets[0].lock();
  if (current != root) {
    return path;
  }

  path.push(current);
  for (auto i = usize(1); i < weak.widgets.size(); ++i) {
    auto widget = weak.widgets[i].lock();
    if (not widget) {
      return truncate ? path : WidgetPath();
    }

    current = find_child(current, widget);
    if (not current) {
      return truncate ? path : WidgetPath();
    }
    path.push(current);
  }

  return path;
}

static auto common_path_length(const WidgetPath& a, const WidgetPath& b)
    -> usize {
  auto common = usize(0);
  while (common < a.size() and common < b.size() and a[common] == b[common]) {
    ++common;
  }
  return common;
}

auto Widget::event(const Event& event) -> EventResponse {
  if (not is_pointer_event(event)) {
    return route_tree_event(event);
  }

  auto hit_test = HitTestList {};
  auto root = shared_from_this();
  hit_test.build_from(root);
  auto path = hit_test.path_at(event.mouse);
  update_hover_path(path, event);
  auto reply = route_pointer_event(event, path);
  update_cursor(event, path);
  return reply;
}

auto Widget::update_hover_path(const WidgetPath& path, const Event& event)
    -> void {
  auto root = shared_from_this();
  auto previous = resolve_weak_path(root, s_pointer_state.hovered_path, true);
  auto hover_path = hover_visible_path(path);
  s_pointer_state.debug_hover_path = debug_path_labels(hover_path);

  auto common = common_path_length(previous, hover_path);

  for (auto i = previous.size(); i > common; --i) {
    auto widget = previous[i - 1];
    widget->m_hovered = false;
    widget->on_mouse_leave(event);
  }

  for (auto i = common; i < hover_path.size(); ++i) {
    auto widget = hover_path[i];
    widget->m_hovered = true;
    widget->on_mouse_enter(event);
  }

  for (const auto& widget : hover_path) { widget->m_hovered = true; }
  s_pointer_state.hovered_path = make_weak_path(hover_path);
}

auto Widget::route_pointer_event(const Event& event, const WidgetPath& path)
    -> EventResponse {
  auto route_to = [&](const SharedPtr<Widget>& widget) -> EventResponse {
    auto reply = EventResponse::unhandled();
    switch (event.kind) {
      case Event::MouseDown:
        reply = widget->apply_event_reply(event, widget->on_mouse_down(event));
        break;
      case Event::MouseUp:
        reply = widget->apply_event_reply(event, widget->on_mouse_up(event));
        break;
      case Event::MouseMove:
        reply = widget->drag_detected_reply(event);
        if (not reply) {
          reply = widget->on_mouse_move(event);
        }
        break;
      default:
        break;
    }

    return reply;
  };

  auto process_capture_reply = [&](const SharedPtr<Widget>& widget,
                                   const WidgetPath& event_path,
                                   EventResponse reply) {
    if (event.kind != Event::MouseDown or not reply.wants_mouse_capture or
        event.button != reply.capture_button) {
      return;
    }

    auto capture_path = path_down_to(event_path, widget);
    if (capture_path.empty()) {
      return;
    }

    s_pointer_state.capture_path = make_weak_path(capture_path);
    s_pointer_state.capture_button = reply.capture_button;
    s_pointer_state.debug_mouse_capture = debug_widget_label(widget, 0);
  };

  auto reply = EventResponse::unhandled();
  auto root = shared_from_this();
  auto capture_path =
      resolve_weak_path(root, s_pointer_state.capture_path, false);
  auto had_captor = not capture_path.empty();
  if (had_captor) {
    auto captor = capture_path[capture_path.size() - 1];
    reply = route_to(captor);
    process_capture_reply(captor, capture_path, reply);
  } else {
    if (not s_pointer_state.capture_path.widgets.empty()) {
      s_pointer_state.capture_path = {};
      s_pointer_state.capture_button = MouseButton::None;
      s_pointer_state.debug_mouse_capture = {};
    }
    for (auto i = path.size(); i > 0; --i) {
      auto widget = path[i - 1];
      reply = route_to(widget);
      process_capture_reply(widget, path, reply);
      if (reply) {
        break;
      }
    }
  }

  if (event.kind == Event::MouseUp and
      event.button == s_pointer_state.capture_button) {
    s_pointer_state.capture_path = {};
    s_pointer_state.capture_button = MouseButton::None;
    s_pointer_state.debug_mouse_capture = {};
  }

  if (reply) {
    return reply;
  }
  if (had_captor) {
    return EventResponse::handled();
  }
  return path.empty() ? EventResponse::unhandled() : EventResponse::handled();
}

auto Widget::update_cursor(const Event& event, const WidgetPath& path) -> void {
  auto shape = cursor_shape_t::Arrow;
  auto root = shared_from_this();
  auto capture_path =
      resolve_weak_path(root, s_pointer_state.capture_path, false);
  if (not capture_path.empty()) {
    auto captor = capture_path[capture_path.size() - 1];
    auto reply = captor->on_cursor_query(event);
    if (reply) {
      Application::get().active_window().set_cursor(reply.shape());
      return;
    }
  } else if (not s_pointer_state.capture_path.widgets.empty()) {
    s_pointer_state.capture_path = {};
    s_pointer_state.capture_button = MouseButton::None;
    s_pointer_state.debug_mouse_capture = {};
  }

  for (auto i = path.size(); i > 0; --i) {
    auto reply = path[i - 1]->on_cursor_query(event);
    if (reply) {
      shape = reply.shape();
      break;
    }
  }

  Application::get().active_window().set_cursor(shape);
}

auto Widget::debug_hover_path() const -> Vector<String> {
  return s_pointer_state.debug_hover_path;
}

auto Widget::debug_mouse_capture() const -> String {
  return s_pointer_state.debug_mouse_capture;
}

auto Widget::route_tree_event(const Event& event) -> EventResponse {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return EventResponse::unhandled();
  }

  auto child_reply = EventResponse::unhandled();
  for (auto child : get_children()) {
    auto reply = child->event(event);
    if (reply) {
      child_reply = reply;
    }
  }

  switch (event.kind) {
    case Event::KeyDown:
      return event_reply(on_key_down(event), child_reply, false);
    case Event::KeyUp:
      return event_reply(on_key_up(event), child_reply, false);
    case Event::KeyRepeat:
      return event_reply(on_key_repeat(event), child_reply, false);
    case Event::WindowResize:
      return event_reply(on_window_resize(event), child_reply, false);
    case Event::WindowClose:
      return event_reply(on_window_close(event), child_reply, false);
    case Event::MouseDown:
    case Event::MouseUp:
    case Event::MouseMove:
    case Event::None:
      break;
  }

  return child_reply;
}

auto Widget::needs_layout() const -> bool {
  if (m_layout_invalidated) {
    return true;
  }

  for (auto child : get_children()) {
    if (child->needs_layout()) {
      return true;
    }
  }
  return false;
}

auto Widget::clear_layout_invalidated() -> void {
  m_layout_invalidated = false;
  for (auto child : get_children()) { child->clear_layout_invalidated(); }
}

auto Widget::apply_event_reply(const Event& event, EventResponse reply)
    -> EventResponse {
  if (event.kind == Event::MouseDown and reply.wants_drag_detection and
      event.button == reply.drag_button and m_hovered) {
    m_detecting_drag = true;
    m_drag_detected = false;
    m_drag_button = reply.drag_button;
    m_drag_start = event.mouse;
  }

  if (event.kind == Event::MouseUp and event.button == m_drag_button) {
    m_detecting_drag = false;
    m_drag_detected = false;
    m_drag_button = MouseButton::None;
  }

  return reply;
}

auto Widget::drag_detected_reply(const Event& event) -> EventResponse {
  if (event.kind != Event::MouseMove or not m_detecting_drag or
      m_drag_detected) {
    return EventResponse::unhandled();
  }

  auto delta = event.mouse - m_drag_start;
  auto moved = delta.x * delta.x + delta.y * delta.y >
               m_drag_threshold * m_drag_threshold;
  if (not moved) {
    return EventResponse::unhandled();
  }

  m_drag_detected = true;
  return on_drag_detected(event);
}

auto Widget::on_mouse_down(const Event& event) -> EventResponse {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->mouse_down) {
      return metadata->mouse_down.execute(event);
    }
  }
  return EventResponse::unhandled();
}

auto Widget::on_mouse_up(const Event& event) -> EventResponse {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->mouse_up) {
      return metadata->mouse_up.execute(event);
    }
  }
  return EventResponse::unhandled();
}

auto Widget::on_mouse_move(const Event& event) -> EventResponse {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->mouse_move) {
      return metadata->mouse_move.execute(event);
    }
  }
  return EventResponse::unhandled();
}

auto Widget::on_mouse_enter(const Event& event) -> void {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->mouse_enter) {
      metadata->mouse_enter.execute(event);
    }
  }
}

auto Widget::on_mouse_leave(const Event& event) -> void {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->mouse_leave) {
      metadata->mouse_leave.execute(event);
    }
  }
}

auto Widget::on_drag_detected(const Event& event) -> EventResponse {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->drag_detected) {
      return metadata->drag_detected.execute(event);
    }
  }
  return EventResponse::unhandled();
}

auto Widget::on_cursor_query(const Event&) const -> CursorReply {
  if (m_has_cursor) {
    return CursorReply::cursor(m_cursor);
  }
  return CursorReply::unhandled();
}

auto Widget::push_self_draws(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }

  if (m_style.background[3] > 0.0f) {
    auto t = std::max(0.0f, m_style.border_thickness);
    auto inner = m_frame.padding(m_style.padding).padding(t);
    rect(inner, m_style.background).paint(cx);
  }
}

} // namespace zod::sodium
