#include "camera.hh"
#include "input.hh"

#define SPEED 0.007f
#define ROT_SPEED 0.8f

namespace zod {

auto Camera::zoom(f32 delta) -> void {
  m_position += m_direction * delta * 0.06f;
}

auto Camera::pan(glm::vec2 delta) -> void {
  auto right = m_right * delta.x * 0.02f;
  auto _up = glm::cross(m_right, m_direction) * delta.y * 0.02f;
  m_position += right;
  m_position += _up;
}

auto Camera::rotate(glm::vec2 delta) -> void {
  auto rot_mat = glm::rotate(m_model, glm::radians(-delta.x), up);
  rot_mat = glm::rotate(rot_mat, glm::radians(-delta.y), m_right);
  m_position = glm::vec3(rot_mat * glm::vec4(m_position, 1.0f));
  m_direction = glm::normalize(-m_position);
}

auto Camera::_update() -> void {
  auto [x, y] = Input::get_mouse_pos();
  glm::vec2 mouse_pos = glm::vec2(x, y);
  glm::vec2 delta = (mouse_pos - m_last_mouse_pos) * 0.28f;
  if (Input::is_key_pressed(GLFW_KEY_RIGHT_ALT) and
      Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
    if (Input::is_key_pressed(GLFW_KEY_RIGHT_CONTROL)) {
      zoom(delta.x);
    } else if (Input::is_key_pressed(GLFW_KEY_RIGHT_SHIFT)) {
      m_panning = true;
      pan(delta);
    }
  } else if (Input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    rotate(delta);
  }
  m_right = glm::cross(up, m_direction);
  update_matrix();
  m_last_mouse_pos = mouse_pos;
  if (not m_panning) {
    m_pan_mouse_pos = mouse_pos;
  }
  m_panning = false;
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
