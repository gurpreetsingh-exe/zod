#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "base/math.hh"
#include "event.hh"

namespace zod {

inline usize widget_id = 0;

struct Geometry {
  std::vector<vec2> points;
  std::vector<vec4> colors;

  auto clear() -> void {
    points.clear();
    colors.clear();
  }
};

class Widget {
public:
  usize id;
  std::string name;
  bool padding = true;

protected:
  bool m_needs_update = true;

public:
  Widget(std::string n) : id(widget_id++), name(std::move(n)) {}
  Widget(std::string n, bool padding_)
      : id(widget_id++), name(std::move(n)), padding(padding_) {}
  virtual ~Widget() = default;
  virtual auto on_event(Event& event) -> void = 0;
  auto draw(Geometry& g) -> void {
    if (not padding) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }
    ImGui::Begin(name.c_str());
    draw_imp(g);
    ImGui::End();
    if (not padding) {
      ImGui::PopStyleVar();
    }
  }

private:
  virtual auto draw_imp(Geometry&) -> void = 0;
};

} // namespace zod
