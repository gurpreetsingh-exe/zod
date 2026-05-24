#pragma once

#include "core/option.hh"
#include "sodium/widgets/compound_widget.hh"

namespace zod::sodium {

class Box : public CompoundWidget {
public:
  Box() = default;
  explicit Box(String name) { set_name(std::move(name)); }

  auto set_width_override(f32 width) -> void { m_width_override = some(width); }
  auto set_height_override(f32 height) -> void {
    m_height_override = some(height);
  }
  auto set_min_desired_width(f32 width) -> void {
    m_min_desired_width = some(width);
  }
  auto set_min_desired_height(f32 height) -> void {
    m_min_desired_height = some(height);
  }
  auto set_max_desired_width(f32 width) -> void {
    m_max_desired_width = some(width);
  }
  auto set_max_desired_height(f32 height) -> void {
    m_max_desired_height = some(height);
  }
  auto set_content_halign(Align align) -> void { m_content_halign = align; }
  auto set_content_valign(Align align) -> void { m_content_valign = align; }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;

private:
  auto constrained_desired_size(vec2, vec2 padding = {}) const -> vec2;
  auto content_alignment(Axis) const -> Align;

private:
  Option<f32> m_width_override;
  Option<f32> m_height_override;
  Option<f32> m_min_desired_width;
  Option<f32> m_min_desired_height;
  Option<f32> m_max_desired_width;
  Option<f32> m_max_desired_height;
  Option<Align> m_content_halign;
  Option<Align> m_content_valign;
};

} // namespace zod::sodium
