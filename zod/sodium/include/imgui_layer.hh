#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "base/math.hh"

namespace zod {

class ImGuiLayer {
public:
  ImGuiLayer(void*);
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
  bool m_open;
  bool m_opt_fullscreen = true;
  bool m_opt_padding = false;
  ImGuiDockNodeFlags m_dockspace_flags = ImGuiDockNodeFlags_None;
  ImGuiWindowFlags m_window_flags = ImGuiWindowFlags_NoDocking;
  vec2 m_dim = { 600, 600 };
};

}; // namespace zod
