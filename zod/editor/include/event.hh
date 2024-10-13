#pragma once

#include <GLFW/glfw3.h>

namespace zod {

struct Event {
  enum EventKind {
    None = 0,
    MouseDown,
    MouseUp,
    MouseMove,

    WindowResize,
  } kind = None;

  enum ButtonKind {
    MouseButtonLeft = 0,
    MouseButtonRight,
    MouseButtonMiddle,
    MouseButtonNone,
  } button = MouseButtonNone;

  float delta_time = 0.f;
  bool shift = false;
  bool ctrl = false;
  bool alt = false;

  union {
    bool pressed[GLFW_KEY_LAST + 1];
    vec2 mouse;
    vec2 size;
  };

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
