#pragma once

#include "sodium/widgets/compound_widget.hh"

namespace zod::sodium {

class DPIScaler : public CompoundWidget {
public:
  explicit DPIScaler(f32 scale = 1.0f) : m_scale(scale) {}

  auto set_scale(f32 scale) -> void { m_scale = scale; }
  auto scale() const -> f32 { return m_scale; }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;

private:
  f32 m_scale = 1.0f;
};

} // namespace zod::sodium
