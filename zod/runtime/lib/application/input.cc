#include <GLFW/glfw3.h>

#include "application/application.hh"
#include "application/input.hh"

namespace zod {

auto Input::is_key_pressed(int key) -> bool { TODO(); }

auto Input::is_mouse_button_pressed(int button) -> bool { TODO(); }

auto Input::get_mouse_pos() -> vec2 {
  return Application::get().active_window().get_mouse_position();
}

auto Input::set_mouse_pos(vec2 position) -> void { TODO(); }

} // namespace zod
