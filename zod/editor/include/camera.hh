#pragma once

#include "event.hh"
#include "input.hh"

namespace zod {

inline constexpr vec3 up = vec3(0.0f, 0.0f, 1.0f);

enum Navigation {
  None = 0,
  Zoom,
  Pan,
  Rotate,
};

class ICamera {
public:
  bool updating = false;

public:
  ICamera(f32 width, f32 height) : m_width(width), m_height(m_height) {}
  virtual ~ICamera() = default;

public:
  auto get_view() const -> const mat4& { return m_view; }
  auto get_projection() const -> const mat4& { return m_projection; }
  auto get_view_projection() const -> const mat4& { return m_view_projection; }
  auto get_direction() const -> const vec3& { return m_direction; }
  auto force_update(bool update) -> void { updating = update; }

  auto set_pivot_point(vec2 v) -> void {
    m_pivot_point = screen_to_world(vec2(v.x, -v.y));
  }
  auto set_pivot_at_mouse() -> void {
    auto mouse = Input::get_mouse_pos();
    set_pivot_point(vec2(mouse.x, mouse.y - m_height));
  }
  auto set_navigation(Navigation n) -> void { m_mode = n; }
  auto get_pivot_point() -> vec2 { return m_pivot_point; }

  auto update_matrix() -> void {
    update_model();
    update_view();
    update_projection();
    m_view_projection = m_projection * m_view;
  }

  auto resize(f32 width, f32 height) -> void {
    m_width = width;
    m_height = height;
    m_needs_update = true;
  }

  auto set_window_position(vec2 position) -> void {
    m_window_position = position;
  }

  auto screen_to_world(vec2 v) -> vec4 {
    auto ndc = vec4((v / vec2(m_width, m_height)) * 2.0f - 1.0f, 0.0f, 1.0f);
    return inverse(m_view_projection) * ndc;
  }

  virtual auto update(Event&) -> bool = 0;

private:
  virtual auto update_model() -> void = 0;
  virtual auto update_view() -> void = 0;
  virtual auto update_projection() -> void = 0;

protected:
  f32 m_width = 0.0f;
  f32 m_height = 0.0f;
  vec2 m_window_position = vec2(0.0f);
  bool m_needs_update = false;
  Navigation m_mode = Navigation::None;
  vec3 m_position = vec3(0.0f, 0.0f, 1.0f);
  vec3 m_direction = normalize(-m_position);
  mat4 m_model = {};
  mat4 m_view = {};
  mat4 m_projection = {};
  mat4 m_view_projection = {};
  vec2 m_pivot_point = {};
};

class OrthographicCamera : public ICamera {
public:
  OrthographicCamera(f32 width, f32 height) : ICamera(width, height) {
    update_matrix();
  }

  ~OrthographicCamera() = default;

public:
  auto update(Event&) -> bool override;
  auto get_zoom() -> f32 { return m_zoom * 0.5f; }

private:
  auto zoom(f32) -> void;
  auto pan(vec2) -> void;
  auto cursor_wrap(vec2) -> void;

  auto update_model() -> void override { m_model = mat4(1.0f); }

  auto update_view() -> void override {
    m_view = lookAt(m_position, m_position + m_direction, vec3(0, 1, 0));
  }

  auto update_projection() -> void override {
    m_projection = ortho(-m_width, m_width, -m_height, m_height, -1.f, 1.f);
  }

private:
  f32 m_zoom = 1.0f;
};

class PerspectiveCamera : public ICamera {
public:
  PerspectiveCamera(f32 width, f32 height, f32 fov, f32 clip_near, f32 clip_far)
      : ICamera(width, height), m_fov(fov), m_clip_near(clip_near),
        m_clip_far(clip_far) {
    m_position = vec3(2.0f, 2.0f, 2.0f);
    m_direction = normalize(-m_position);
    update_matrix();
  }

  ~PerspectiveCamera() = default;

public:
  auto get_fov() const -> f32 { return m_fov; }
  auto get_clipping() const -> std::tuple<f32, f32> {
    return { m_clip_near, m_clip_far };
  }

  auto update(Event&) -> bool override;

  auto set_fov(f32 fov) -> void {
    m_fov = fov;
    m_needs_update = true;
  }

  auto set_clipping(f32 near, f32 far) -> void {
    m_clip_near = near;
    m_clip_far = far;
    m_needs_update = true;
  }

private:
  auto update_model() -> void override { m_model = mat4(1.0f); }

  auto update_view() -> void override {
    m_view = lookAt(m_position, m_position + m_direction, up);
  }

  auto update_projection() -> void override {
    m_projection = perspective(radians(m_fov), m_width / m_height, m_clip_near,
                               m_clip_far);
  }

  auto zoom(f32) -> void;
  auto rotate(vec2) -> void;
  auto pan(vec2) -> void;
  auto cursor_wrap(vec2) -> void;

private:
  f32 m_fov;
  f32 m_clip_near;
  f32 m_clip_far;

  bool m_look_around = false;
  vec3 m_right = vec3(1.0f, 0.0f, 0.0f);
};

} // namespace zod

FMT(zod::Navigation, "{}", [&] {
  switch (v) {
    case zod::Navigation::None:
      return "None";
    case zod::Navigation::Zoom:
      return "Zoom";
    case zod::Navigation::Pan:
      return "Pan";
    case zod::Navigation::Rotate:
      return "Rotate";
  }
}());
