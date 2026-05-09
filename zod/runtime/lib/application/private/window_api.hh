#pragma once

#include "core/platform/macros.hh"

#if defined(PLATFORM_LINUX)
#  define GLFW_EXPOSE_NATIVE_X11
#  include <dlfcn.h>
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace x11 {

auto init_x11() -> void;
auto send_window_event(Display*, Window, int, int, int) -> void;
auto get_resize_direction(zod::f64, zod::f64, int, int) -> int;
auto mouse_down_event(GLFWwindow*, zod::f64, zod::f64, int, int) -> void;

#undef None

} // namespace x11
