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

SceneCamera::SceneCamera(f32 width, f32 height, f32 fov, f32 clip_near,
                         f32 clip_far)
    : PerspectiveCamera(width, height, fov, clip_near, clip_far) {
  auto offset = m_position - m_orbit_pivot;
  m_orbit_distance = glm::length(offset);
  m_yaw = glm::atan(offset.y, offset.x);
  m_pitch = glm::asin(offset.z / m_orbit_distance);
  rebuild();
  update_matrix();
}

auto SceneCamera::set_orbit_pivot(vec3 pivot) -> void {
  m_orbit_pivot = pivot;
  rebuild();
  update_matrix();
  is_dirty = true;
}

auto SceneCamera::zoom(f32 delta) -> void {
  m_orbit_distance = std::max(0.01f, m_orbit_distance - delta * 0.06f);
}

auto SceneCamera::pan(vec2 delta) -> void {
  auto scale = m_orbit_distance * 0.004f;
  m_view_pan -= vec2(delta.x, -delta.y) * scale;
}

auto SceneCamera::rotate(vec2 delta) -> void {
  m_yaw += radians(-delta.x);
  m_pitch += radians(delta.y);
  m_pitch = std::clamp(m_pitch, radians(-89.0f), radians(89.0f));
}

auto SceneCamera::rebuild() -> void {
  auto cp = glm::cos(m_pitch);
  auto offset =
      vec3(glm::cos(m_yaw) * cp, glm::sin(m_yaw) * cp, glm::sin(m_pitch));
  offset *= m_orbit_distance;

  auto orbit_position = m_orbit_pivot + offset;
  auto forward = normalize(m_orbit_pivot - orbit_position);
  auto right = normalize(cross(forward, up));
  auto camera_up = normalize(cross(right, forward));
  auto pan = right * m_view_pan.x + camera_up * m_view_pan.y;

  m_position = orbit_position + pan;
  m_target = m_orbit_pivot + pan;
  m_direction = normalize(m_target - m_position);
}

auto SceneCamera::update(const Event& event) -> bool {
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

  rebuild();
  update_matrix();
  return is_dirty;
}

}; // namespace zod
