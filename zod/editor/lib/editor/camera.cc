#include "camera.hh"
#include "input.hh"

#define SPEED 0.007f
#define ROT_SPEED 0.8f

namespace zod {

auto Camera::_update(Event& event) -> void {
  glm::vec2 mouse_pos = glm::vec2(event.mouse[0], event.mouse[1]);
  glm::vec2 delta = (mouse_pos - m_last_mouse_pos) * 0.28f;
  auto ac = Input::is_key_pressed(GLFW_KEY_RIGHT_ALT) and
            Input::is_key_pressed(GLFW_KEY_RIGHT_CONTROL);
  if (ac and event.button == Event::MouseButtonLeft) {
    m_position += m_direction * delta.x * 0.06f;
  } else if (event.button == Event::MouseButtonRight) {
    glm::mat4 rot_mat = glm::rotate(m_model, glm::radians(delta.x), up);
    rot_mat = glm::rotate(rot_mat, glm::radians(delta.y), m_right);
    m_position = glm::vec3(glm::vec4(m_position, 1.0f) * rot_mat);
    m_direction = glm::normalize(-m_position);
    m_right = glm::cross(up, m_direction);
  }
  update_matrix();
  m_last_mouse_pos = mouse_pos;
  // m_uniform_buffer->upload_data(glm::value_ptr(m_view_projection),
  //                              sizeof(glm::mat4), sizeof(glm::mat4));
}

auto Camera::update(Event& event) -> void {
  if (m_needs_update) {
    m_needs_update = false;
    _update(event);
    return;
  }

  glm::vec2 mouse_pos = glm::vec2(event.mouse[0], event.mouse[1]);
  if (event.kind == Event::None || m_last_mouse_pos == mouse_pos) {
    return;
  }

  if (event.kind != Event::MouseMove) {
    m_last_mouse_pos = mouse_pos;
    return;
  }

  _update(event);
}

}; // namespace zod
