#include "camera.hh"
#include "input.hh"

#define SPEED 0.007f
#define ROT_SPEED 0.8f

namespace zod {

auto Camera::_update() -> void {
  auto [x, y] = Input::get_mouse_pos();
  glm::vec2 mouse_pos = glm::vec2(x, y);
  glm::vec2 delta = (mouse_pos - m_last_mouse_pos) * 0.28f;
  if (Input::is_key_pressed(GLFW_KEY_RIGHT_ALT) and
      Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
    if (Input::is_key_pressed(GLFW_KEY_RIGHT_CONTROL)) {
      m_position += m_direction * delta.x * 0.06f;
    }
  } else if (Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    glm::mat4 rot_mat = glm::rotate(m_model, glm::radians(delta.x), up);
    rot_mat = glm::rotate(rot_mat, glm::radians(delta.y), m_right);
    m_position = glm::vec3(glm::vec4(m_position, 1.0f) * rot_mat);
  }
  m_direction = glm::normalize(-m_position);
  m_right = glm::cross(up, m_direction);
  update_matrix();
  m_last_mouse_pos = mouse_pos;
}

auto Camera::update() -> void {
  if (m_needs_update) {
    m_needs_update = false;
    _update();
    return;
  }

  auto [x, y] = Input::get_mouse_pos();
  glm::vec2 mouse_pos = glm::vec2(x, y);
  if (m_last_mouse_pos == mouse_pos) {
    return;
  }

  _update();
}

}; // namespace zod
