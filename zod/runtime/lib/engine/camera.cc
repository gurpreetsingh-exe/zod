#include "engine/camera.hh"

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

auto OrthographicCamera::update(const Event& event) -> bool {
  vec2 delta = event.mouse - event.last_mouse;
  switch (m_mode) {
    case Navigation::Zoom: {
      zoom(delta.x);
      is_dirty = true;
    } break;
    case Navigation::Pan: {
      pan(delta);
      is_dirty = true;
    } break;
    case Navigation::None:
    case Navigation::Rotate:
    default:
      break;
  }
  update_projection();
  m_view_projection = m_projection * m_view;
  return is_dirty;
}

auto PerspectiveCamera::zoom(f32 delta) -> void {
  auto movement = m_direction * delta * 0.06f;
  m_position += movement;
  m_target += movement;
}

auto PerspectiveCamera::pan(vec2 delta) -> void {
  auto right = m_right * delta.x * 0.02f;
  auto _up = cross(m_right, m_direction) * -delta.y * 0.02f;
  auto movement = right + _up;
  m_position += movement;
  m_target += movement;
}

auto PerspectiveCamera::rotate(vec2 delta) -> void {
  auto offset = m_position - m_target;
  auto rot_mat = zod::rotate(m_model, radians(-delta.x), up);
  rot_mat = zod::rotate(rot_mat, radians(delta.y), m_right);
  offset = vec3(rot_mat * vec4(offset, 1.0f));
  m_position = m_target + offset;
  m_direction = normalize(m_target - m_position);
}

auto PerspectiveCamera::update(const Event& event) -> bool {
  vec2 delta = (event.mouse - event.last_mouse) * 0.28f;
  switch (m_mode) {
    case Navigation::Zoom: {
      zoom(delta.x);
      is_dirty = true;
    } break;
    case Navigation::Pan: {
      pan(delta);
      is_dirty = true;
    } break;
    case Navigation::Rotate: {
      rotate(delta);
      is_dirty = true;
    } break;
    case Navigation::None:
    default:
      break;
  }
  m_right = cross(up, m_direction);
  update_matrix();
  return is_dirty;
}

}; // namespace zod
