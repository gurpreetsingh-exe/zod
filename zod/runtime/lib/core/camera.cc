#include "core/camera.hh"

#define SPEED 0.007f
#define ROT_SPEED 0.8f

namespace zod {

auto OrthographicCamera::zoom(f32 delta) -> void {
  auto zoom = delta * 0.002f;
  m_zoom *= 1.0f + zoom;

  auto coords = vec4(m_pivot_point, 0.0f, 1.0f);
  m_view = translate(m_view, vec3(coords));
  m_view = scale(m_view, 1.0f + vec3(zoom, zoom, 0.0f));
  m_view = translate(m_view, -vec3(coords));
}

auto OrthographicCamera::pan(vec2 delta) -> void {
  m_view = translate(m_view, vec3(delta.x, -delta.y, 0.0f) / get_zoom());
}

auto OrthographicCamera::update(Event& event) -> bool {
  vec2 delta = event.mouse - event.last_mouse;
  bool update = false;
  switch (m_mode) {
    case Navigation::Zoom: {
      zoom(delta.x);
      update = true;
    } break;
    case Navigation::Pan: {
      pan(delta);
      update = true;
    } break;
    case Navigation::None:
    case Navigation::Rotate:
    default:
      break;
  }
  update_projection();
  m_view_projection = m_projection * m_view;
  return update;
}

auto PerspectiveCamera::zoom(f32 delta) -> void {
  m_position += m_direction * delta * 0.06f;
}

auto PerspectiveCamera::pan(vec2 delta) -> void {
  auto right = m_right * delta.x * 0.02f;
  auto _up = cross(m_right, m_direction) * -delta.y * 0.02f;
  m_position += right;
  m_position += _up;
}

auto PerspectiveCamera::rotate(vec2 delta) -> void {
  auto rot_mat = zod::rotate(m_model, radians(-delta.x), up);
  rot_mat = zod::rotate(rot_mat, radians(delta.y), m_right);
  m_position = vec3(rot_mat * vec4(m_position, 1.0f));
  m_direction = normalize(-m_position);
}

auto PerspectiveCamera::update(Event& event) -> bool {
  vec2 delta = (event.mouse - event.last_mouse) * 0.28f;
  bool update = false;
  switch (m_mode) {
    case Navigation::Zoom: {
      zoom(delta.x);
      update = true;
    } break;
    case Navigation::Pan: {
      pan(delta);
      update = true;
    } break;
    case Navigation::Rotate: {
      rotate(delta);
      update = true;
    } break;
    case Navigation::None:
    default:
      break;
  }
  m_right = cross(up, m_direction);
  update_matrix();
  return update;
}

}; // namespace zod
