#pragma once

#include "backend.hh"
#include "event.hh"

namespace zod {

inline constexpr vec3 up = vec3(0.0f, 0.0f, 1.0f);

class OrthographicCamera {
public:
  bool updating = false;
  OrthographicCamera(f32 width, f32 height) : m_width(width), m_height(height) {
    update_matrix();
  }

  ~OrthographicCamera() = default;

public:
  auto get_view_projection() const -> const mat4& { return m_view_projection; }

  auto resize(f32 width, f32 height) -> void {
    m_width = width;
    m_height = height;
    m_needs_update = true;
  }

  auto set_window_position(vec2 position) -> void {
    m_window_position = position;
  }

  auto update() -> void;
  auto get_zoom() -> f32 { return m_zoom * 0.5f; }

private:
  auto _update() -> void;
  auto zoom(f32) -> void;
  auto pan(vec2) -> void;
  auto cursor_wrap(vec2) -> void;
  auto update_matrix() -> void {
    update_view();
    update_projection();
    m_view_projection = m_projection * m_view;
  }

  auto update_view() -> void {
    m_view = lookAt(m_position, m_position + m_direction, vec3(0, 1, 0));
  }

  auto update_projection() -> void {
    m_projection = ortho(-m_width, m_width, -m_height, m_height, -1.f, 1.f);
  }

  auto screen_to_world(vec2 v) -> vec4 {
    auto ndc = vec4((v / vec2(m_width, m_height)) * 2.0f - 1.0f, 0.0f, 0.0f);
    return inverse(m_view_projection) * ndc;
  }

private:
  f32 m_width;
  f32 m_height;
  f32 m_zoom = 1.0f;
  vec3 m_position = vec3(0.0f, 0.0f, 1.0f);
  vec3 m_direction = normalize(-m_position);
  mat4 m_view;
  mat4 m_projection;
  mat4 m_view_projection;
  bool m_needs_update = false;
  bool m_panning = false;
  vec2 m_last_mouse_pos;
  vec2 m_pan_mouse_pos;
  vec2 m_window_position = vec2(0.0f);
};

class Camera {
public:
  bool updating = false;

public:
  Camera(f32 width, f32 height, f32 fov, f32 clip_near, f32 clip_far)
      : m_viewport_width(width), m_viewport_height(height), m_fov(fov),
        m_clip_near(clip_near), m_clip_far(clip_far) {
    update_model();
    update_projection();
    update_view();
  }

  ~Camera() = default;

public:
  auto get_model() const -> const mat4& { return m_model; }
  auto get_view() const -> const mat4& { return m_view; }
  auto get_projection() const -> const mat4& { return m_projection; }
  auto get_view_projection() const -> const mat4& { return m_view_projection; }
  auto get_fov() const -> f32 { return m_fov; }
  auto get_clipping() const -> std::tuple<f32, f32> {
    return { m_clip_near, m_clip_far };
  }
  auto get_direction() const -> const vec3& { return m_direction; }

  auto update() -> void;

  auto resize(f32 width, f32 height) -> void {
    m_viewport_width = width;
    m_viewport_height = height;
    m_needs_update = true;
  }

  auto set_fov(f32 fov) -> void {
    m_fov = fov;
    m_needs_update = true;
  }

  auto set_clipping(f32 near, f32 far) -> void {
    m_clip_near = near;
    m_clip_far = far;
    m_needs_update = true;
  }

  auto set_window_position(vec2 position) -> void {
    m_window_position = position;
  }

private:
  auto update_model() -> void { m_model = mat4(1.0f); }

  auto update_view() -> void {
    m_view = lookAt(m_position, m_position + m_direction, up);
  }

  auto update_projection() -> void {
    m_projection =
        perspective(radians(m_fov), m_viewport_width / m_viewport_height,
                    m_clip_near, m_clip_far);
  }

  auto update_matrix() -> void {
    update_view();
    update_projection();
    m_view_projection = m_projection * m_view;
  }

  auto _update() -> void;
  auto zoom(f32) -> void;
  auto rotate(vec2) -> void;
  auto pan(vec2) -> void;
  auto cursor_wrap(vec2) -> void;

private:
  f32 m_viewport_width, m_viewport_height;
  mat4 m_model;
  mat4 m_view;
  mat4 m_projection;
  mat4 m_view_projection;

  f32 m_fov;
  f32 m_clip_near;
  f32 m_clip_far;

  bool m_needs_update = false;
  bool m_look_around = false;
  bool m_panning = false;

  vec3 m_position = vec3(2.0f, 2.0f, 2.0f);
  vec3 m_direction = normalize(-m_position);
  vec3 m_right = vec3(1.0f, 0.0f, 0.0f);

  vec2 m_last_mouse_pos = vec2(0.0f);
  vec2 m_pan_mouse_pos = vec2(0.0f);

  vec2 m_window_position = vec2(0.0f);
};

} // namespace zod
