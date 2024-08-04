#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "base/math.hh"

namespace zod {

class ImGuiLayer {
public:
  ImGuiLayer(GLFWwindow*);
  ~ImGuiLayer();

public:
  template <typename Callable>
  auto update(Callable cb) -> void {
    cb();
  }

  auto begin_frame() -> void;
  auto end_frame() -> void;
  auto get_viewport_dimensions() -> vec2 { return m_dim; }

private:
  vec2 m_dim = { 600, 600 };
};

}; // namespace zod
