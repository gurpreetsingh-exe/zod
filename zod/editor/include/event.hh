#pragma once

#include "input.hh"

namespace zod {

enum class MouseButton {
  Left = 0,
  Right,
  Middle,
  None,
};

struct Event {
  enum EventKind {
    None = 0,
    MouseDown,
    MouseUp,
    MouseMove,

    KeyDown,
    KeyUp,
    KeyRepeat,

    WindowResize,
  } kind = None;

  MouseButton button = MouseButton::None;

  float delta_time = 0.f;
  bool shift = false;
  bool ctrl = false;
  bool alt = false;

  union {
    bool pressed[GLFW_KEY_LAST + 1];
    vec2 mouse;
    vec2 size;
    Key key;
  };

  auto is_key_down(Key k) const -> bool { return kind == KeyDown and k == key; }

  auto is_mouse_down(MouseButton b) const -> bool {
    return kind == MouseDown and b == button;
  }

  const char* name() {
    switch (kind) {
      case MouseMove:
        return "MouseMove";
      case MouseDown:
        return "MouseDown";
      case MouseUp:
        return "MouseUp";
      case WindowResize:
        return "WindowResize";
      case None:
        return "None";
      default:
        UNREACHABLE();
    }
  }
};

}; // namespace zod
