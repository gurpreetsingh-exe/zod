#pragma once

#include "core/base/math.hh"
#include "core/event.hh"

namespace zod {

inline usize widget_id = 0;

struct Geometry {
  std::vector<vec2> points;
  std::vector<vec4> colors;
  std::vector<u32> indices;

  auto clear() -> void {
    points.clear();
    colors.clear();
    indices.clear();
  }
};

class SWidget {
public:
  usize id;
  std::string name;

protected:
  bool m_needs_update = true;
  vec2 m_position = {};
  vec2 m_size = {};
  SWidget* m_parent = nullptr;

public:
  SWidget(std::string n) : id(widget_id++), name(std::move(n)) {}
  virtual ~SWidget() = default;
  virtual auto on_event(Event& event) -> void = 0;
  virtual auto compute_desired_size() -> void = 0;
  auto get_position() const -> vec2 { return m_position; }
  auto get_size() const -> vec2 { return m_size; }
  auto set_position(vec2 position) -> void { m_position = position; }
  auto set_size(vec2 size) -> void { m_size = size; }
  auto draw(Geometry& g) -> void { draw_imp(g); }

protected:
  virtual auto draw_imp(Geometry&) -> void = 0;
};

} // namespace zod
