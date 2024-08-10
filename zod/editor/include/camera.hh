#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "backend.hh"
#include "event.hh"

namespace zod {

inline constexpr glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

class Camera {
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

  auto set_clipping(f32 near, f32 far) {
    m_clip_near = near;
    m_clip_far = far;
    m_needs_update = true;
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

  glm::vec3 m_position = glm::vec3(2.0f, 2.0f, 2.0f);
  glm::vec3 m_direction = glm::vec3(-1.0f);
  glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);

  glm::vec2 m_last_mouse_pos = glm::vec2(0.0f);
};

} // namespace zod