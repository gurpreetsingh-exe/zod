#include "input.hh"
#include "application/context.hh"

namespace zod {

auto Input::is_key_pressed(int key) -> bool {
  auto window = ZCtxt::get().get_window().get_handle();
  auto state = glfwGetKey(window, key);
  return state == GLFW_PRESS or state == GLFW_REPEAT;
}

auto Input::is_mouse_button_pressed(int button) -> bool {
  auto window = ZCtxt::get().get_window().get_handle();
  return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

} // namespace zod
