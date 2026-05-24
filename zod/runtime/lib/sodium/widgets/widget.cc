#include "sodium/widgets/widget.hh"

namespace zod::sodium {

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

auto WidgetPath::contains(const Widget* widget) const -> bool {
  return std::find(m_widgets.begin(), m_widgets.end(), widget) !=
         m_widgets.end();
}

static auto is_pointer_event(const Event& event) -> bool {
  return event.kind == Event::MouseDown or event.kind == Event::MouseUp or
         event.kind == Event::MouseMove;
}

auto Widget::event(const Event& event) -> EventResponse {
  if (not is_pointer_event(event)) {
    return route_tree_event(event);
  }

  auto path = WidgetPath {};
  find_path_at(event.mouse, path);
  update_hover_path(path, event);
  return route_pointer_event(event, path);
}

auto Widget::find_path_at(vec2 mouse, WidgetPath& path) -> bool {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return false;
  }

  if (not m_frame.intersect(mouse)) {
    return false;
  }

  auto start = path.size();
  auto self_hit_testable = m_visibility != Visibility::SelfHitTestInvisible;
  if (self_hit_testable) {
    path.push(this);
  }

  auto children = get_children();
  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    if (*it and (*it)->find_path_at(mouse, path)) {
      return true;
    }
  }

  if (self_hit_testable) {
    return true;
  }

  path.resize(start);
  return false;
}

auto Widget::update_hover_path(const WidgetPath& path, const Event& event)
    -> void {
  auto common = usize(0);
  while (common < m_hovered_path.size() and common < path.size() and
         m_hovered_path[common] == path[common]) {
    ++common;
  }

  for (auto i = m_hovered_path.size(); i > common; --i) {
    auto* widget = m_hovered_path[i - 1];
    widget->m_hovered = false;
    widget->on_mouse_leave(event);
  }

  for (auto i = common; i < path.size(); ++i) {
    auto* widget = path[i];
    widget->m_hovered = true;
    widget->on_mouse_enter(event);
  }

  for (auto* widget : path) { widget->m_hovered = true; }
  m_hovered_path = path;
}

auto Widget::route_pointer_event(const Event& event, const WidgetPath& path)
    -> EventResponse {
  auto route_to = [&](Widget* widget) {
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

    if (event.kind == Event::MouseDown and reply.wants_mouse_capture and
        event.button == reply.capture_button) {
      m_mouse_captor = widget;
      m_mouse_capture_button = reply.capture_button;
    }

    return reply;
  };

  auto reply = EventResponse::unhandled();
  auto had_captor = m_mouse_captor != nullptr;
  if (m_mouse_captor) {
    reply = route_to(m_mouse_captor);
  } else {
    for (auto i = path.size(); i > 0; --i) {
      reply = route_to(path[i - 1]);
      if (reply) {
        break;
      }
    }
  }

  if (event.kind == Event::MouseUp and event.button == m_mouse_capture_button) {
    m_mouse_captor = nullptr;
    m_mouse_capture_button = MouseButton::None;
  }

  if (reply) {
    return reply;
  }
  if (had_captor) {
    return EventResponse::handled();
  }
  return path.empty() ? EventResponse::unhandled() : EventResponse::handled();
}

auto Widget::route_tree_event(const Event& event) -> EventResponse {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return EventResponse::unhandled();
  }

  auto child_reply = EventResponse::unhandled();
  for (auto child : get_children()) {
    if (child) {
      auto reply = child->event(event);
      if (reply) {
        child_reply = reply;
      }
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
    if (child and child->needs_layout()) {
      return true;
    }
  }
  return false;
}

auto Widget::clear_layout_invalidated() -> void {
  m_layout_invalidated = false;
  for (auto child : get_children()) {
    if (child) {
      child->clear_layout_invalidated();
    }
  }
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
