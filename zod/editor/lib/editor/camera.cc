#include "camera.hh"

#define SPEED 0.007f
#define ROT_SPEED 0.8f

namespace zod {

auto OrthographicCamera::cursor_wrap(vec2 position) -> void {
  constexpr f32 padding = 16;
  if (position.x < m_window_position.x + padding) {
    auto pos =
        vec2(m_window_position.x + m_width - padding, Input::get_mouse_pos().y);
    Input::set_mouse_pos(pos);
  } else if (position.x > m_window_position.x + m_width - padding) {
    auto pos = vec2(m_window_position.x + padding, Input::get_mouse_pos().y);
    Input::set_mouse_pos(pos);
  }
}

auto OrthographicCamera::zoom(f32 delta) -> void {
  auto zoom = delta * 0.002f;
  m_zoom *= 1.0f + zoom;

  auto coords = vec4(m_pivot_point, 0.0f, 1.0f);
  m_view = translate(m_view, vec3(coords));
  m_view = scale(m_view, 1.0f + vec3(zoom, zoom, 0.0f));
  m_view = translate(m_view, -vec3(coords));
}

auto OrthographicCamera::pan(vec2 delta) -> void {
  m_view = translate(m_view, vec3(delta / get_zoom(), 0.0f));
}

auto OrthographicCamera::_update() -> void {
  auto mouse_pos = Input::get_mouse_pos();
  vec2 delta = mouse_pos - m_last_mouse_pos;
  switch (m_mode) {
    case Navigation::Zoom: {
      zoom(delta.x);
      cursor_wrap(mouse_pos);
    } break;
    case Navigation::Pan: {
      pan(delta);
      cursor_wrap(mouse_pos);
    } break;
    case Navigation::None:
    case Navigation::Rotate:
    default:
      break;
  }
  update_projection();
  m_view_projection = m_projection * m_view;
  m_last_mouse_pos = Input::get_mouse_pos();
}

auto OrthographicCamera::update() -> void {
  if (m_needs_update) {
    m_needs_update = false;
    _update();
    return;
  }

  if (m_last_mouse_pos == Input::get_mouse_pos()) {
    return;
  }

  if (not updating) {
    return;
  }

  _update();
}

auto PerspectiveCamera::zoom(f32 delta) -> void {
  m_position += m_direction * delta * 0.06f;
}

auto PerspectiveCamera::pan(vec2 delta) -> void {
  auto right = m_right * delta.x * 0.02f;
  auto _up = cross(m_right, m_direction) * delta.y * 0.02f;
  m_position += right;
  m_position += _up;
}

auto PerspectiveCamera::rotate(vec2 delta) -> void {
  auto rot_mat = zod::rotate(m_model, radians(-delta.x), up);
  rot_mat = zod::rotate(rot_mat, radians(-delta.y), m_right);
  m_position = vec3(rot_mat * vec4(m_position, 1.0f));
  m_direction = normalize(-m_position);
}

auto PerspectiveCamera::cursor_wrap(vec2 position) -> void {
  constexpr f32 padding = 16;
  if (position.x < m_window_position.x + padding) {
    auto pos =
        vec2(m_window_position.x + m_width - padding, Input::get_mouse_pos().y);
    Input::set_mouse_pos(pos);
  } else if (position.x > m_window_position.x + m_width - padding) {
    auto pos = vec2(m_window_position.x + padding, Input::get_mouse_pos().y);
    Input::set_mouse_pos(pos);
  }
}

auto PerspectiveCamera::_update() -> void {
  auto mouse_pos = Input::get_mouse_pos();
  vec2 delta = (mouse_pos - m_last_mouse_pos) * 0.28f;
  switch (m_mode) {
    case Navigation::Zoom: {
      zoom(delta.x);
      cursor_wrap(mouse_pos);
    } break;
    case Navigation::Pan: {
      pan(delta);
      cursor_wrap(mouse_pos);
    } break;
    case Navigation::Rotate: {
      rotate(delta);
      cursor_wrap(mouse_pos);
    } break;
    case Navigation::None:
    default:
      break;
  }
  m_right = cross(up, m_direction);
  update_matrix();
  m_last_mouse_pos = Input::get_mouse_pos();
}

auto PerspectiveCamera::update() -> void {
  if (m_needs_update) {
    m_needs_update = false;
    _update();
    return;
  }

  if (m_last_mouse_pos == Input::get_mouse_pos()) {
    return;
  }

  if (not updating) {
    return;
  }

  _update();
}

}; // namespace zod
