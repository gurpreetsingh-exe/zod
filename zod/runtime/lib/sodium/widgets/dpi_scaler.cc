#include "sodium/widgets/dpi_scaler.hh"

namespace zod::sodium {

auto DPIScaler::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  auto safe_scale = std::max(0.0001f, m_scale);
  auto unscaled = vec2 { available.x / safe_scale, available.y / safe_scale };
  auto desired = CompoundWidget::compute_desired_size(unscaled);
  return cache_desired_size(desired * safe_scale);
}

auto DPIScaler::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  if (not m_child_slot.child) {
    return;
  }

  auto safe_scale = std::max(0.0001f, m_scale);
  auto scaled_bounds = Rect { bounds.position, bounds.size / safe_scale };
  CompoundWidget::arrange(scaled_bounds);
}

} // namespace zod::sodium
