#include "application/event.hh"

namespace zod {

auto Event::name() -> const char* {
  switch (kind) {
    case MouseMove:
      return "MouseMove";
    case MouseDown:
      return "MouseDown";
    case MouseUp:
      return "MouseUp";
    case KeyDown:
      return "KeyDown";
    case KeyUp:
      return "KeyUp";
    case KeyRepeat:
      return "KeyRepeat";
    case WindowResize:
      return "WindowResize";
    case WindowClose:
      return "WindowClose";
    case None:
      return "None";
    default:
      UNREACHABLE();
  }
}

auto EventResponse::detect_drag(MouseButton button) -> EventResponse {
  wants_drag_detection = true;
  drag_button = button;
  is_handled = true;
  return *this;
}

auto EventResponse::capture_mouse(MouseButton button) -> EventResponse {
  wants_mouse_capture = true;
  capture_button = button;
  is_handled = true;
  return *this;
}

} // namespace zod
