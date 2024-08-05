#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

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
    f32 mouse[2];
    f32 size[2];
  };

  const char* name() {
    switch (kind) {
      case MouseMove:
        return "MouseMove";
      case MouseDown:
        return "MouseDown";
      case MouseUp:
        return "MouseUp";
      case None:
        return "None";
      default:
        UNREACHABLE();
    }
  }
};

}; // namespace zod
