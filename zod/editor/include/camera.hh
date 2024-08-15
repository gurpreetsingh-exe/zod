#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "backend.hh"
#include "event.hh"

#define FMT(type, fmt_str, ...)                                                \
  template <>                                                                  \
  struct fmt::formatter<type> {                                                \
    constexpr auto parse(format_parse_context& ctx)                            \
        -> format_parse_context::iterator {                                    \
      return ctx.begin();                                                      \
    }                                                                          \
    auto format(type v, format_context& ctx) const                             \
        -> format_context::iterator {                                          \
      return fmt::format_to(ctx.out(), fmt_str, __VA_ARGS__);                  \
    }                                                                          \
  }

FMT(glm::vec3, "{{ {}, {}, {} }}", v.x, v.y, v.z);
FMT(glm::vec4, "{{ {}, {}, {}, {} }}", v.x, v.y, v.z, v.w);
FMT(glm::mat4, "{{ {},\n  {},\n  {},\n  {} }}", v[0], v[1], v[2], v[3]);

namespace zod {

inline constexpr glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

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
  auto get_model() const -> const glm::mat4& { return m_model; }
  auto get_view() const -> const glm::mat4& { return m_view; }
  auto get_projection() const -> const glm::mat4& { return m_projection; }
  auto get_view_projection() const -> const glm::mat4& {
    return m_view_projection;
  }
  auto get_fov() const -> f32 { return m_fov; }
  auto get_clipping() const -> std::tuple<f32, f32> {
    return { m_clip_near, m_clip_far };
  }
  auto get_direction() const -> const glm::vec3& { return m_direction; }

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

  auto set_window_position(glm::vec2 position) -> void {
    m_window_position = position;
  }

private:
  auto update_model() -> void { m_model = glm::mat4(1.0f); }

  auto update_view() -> void {
    m_view = glm::lookAt(m_position, m_position + m_direction, up);
  }

  auto update_projection() -> void {
    m_projection = glm::perspective(glm::radians(m_fov),
                                    m_viewport_width / m_viewport_height,
                                    m_clip_near, m_clip_far);
  }

  auto update_matrix() -> void {
    update_view();
    update_projection();
    m_view_projection = m_projection * m_view;
  }

  auto _update() -> void;
  auto zoom(f32) -> void;
  auto rotate(glm::vec2) -> void;
  auto pan(glm::vec2) -> void;
  auto cursor_wrap(glm::vec2) -> void;

private:
  f32 m_viewport_width, m_viewport_height;
  glm::mat4 m_model;
  glm::mat4 m_view;
  glm::mat4 m_projection;
  glm::mat4 m_view_projection;

  f32 m_fov;
  f32 m_clip_near;
  f32 m_clip_far;

  bool m_needs_update = false;
  bool m_look_around = false;
  bool m_panning = false;

  glm::vec3 m_position = glm::vec3(2.0f, 2.0f, 2.0f);
  glm::vec3 m_direction = glm::normalize(-m_position);
  glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);

  glm::vec2 m_last_mouse_pos = glm::vec2(0.0f);
  glm::vec2 m_pan_mouse_pos = glm::vec2(0.0f);

  glm::vec2 m_window_position = glm::vec2(0.0f);
};

} // namespace zod
