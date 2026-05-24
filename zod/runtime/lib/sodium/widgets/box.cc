#include "sodium/widgets/box.hh"

namespace zod::sodium {

auto Box::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  auto desired = CompoundWidget::compute_desired_size(available);
  auto padding = vec2 {};
  if (m_child_slot.child) {
    padding =
        m_style.padding.combined() + m_child_slot.style.padding.combined();
    if (m_child_slot.style.horizontal_rule == SizeRule::Fixed) {
      desired.x = m_child_slot.style.size.x + padding.x;
    }
    if (m_child_slot.style.vertical_rule == SizeRule::Fixed) {
      desired.y = m_child_slot.style.size.y + padding.y;
    }
  }
  if (m_desired_size.x > 0.0f) {
    desired.x = m_desired_size.x;
  }
  if (m_desired_size.y > 0.0f) {
    desired.y = m_desired_size.y;
  }
  return cache_desired_size(constrained_desired_size(desired, padding));
}

auto Box::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  if (not m_child_slot.child) {
    return;
  }

  auto child_outer =
      m_frame.padding(m_style.padding).padding(m_child_slot.style.padding);
  auto desired = m_child_slot.child->desired_size();
  if (m_child_slot.style.horizontal_rule == SizeRule::Fixed) {
    desired.x = m_child_slot.style.size.x;
  }
  if (m_child_slot.style.vertical_rule == SizeRule::Fixed) {
    desired.y = m_child_slot.style.size.y;
  }
  desired = constrained_desired_size(desired);

  auto aligned =
      align_within(child_outer, desired, content_alignment(Axis::Horizontal),
                   content_alignment(Axis::Vertical));
  m_child_slot.child->arrange(aligned);
}

auto Box::constrained_desired_size(vec2 desired, vec2 padding) const -> vec2 {
  if (m_width_override) {
    desired.x = *m_width_override + padding.x;
  }
  if (m_height_override) {
    desired.y = *m_height_override + padding.y;
  }
  if (m_min_desired_width) {
    desired.x = std::max(desired.x, *m_min_desired_width + padding.x);
  }
  if (m_min_desired_height) {
    desired.y = std::max(desired.y, *m_min_desired_height + padding.y);
  }
  if (m_max_desired_width) {
    desired.x = std::min(desired.x, *m_max_desired_width + padding.x);
  }
  if (m_max_desired_height) {
    desired.y = std::min(desired.y, *m_max_desired_height + padding.y);
  }
  return desired;
}

auto Box::content_alignment(Axis axis) const -> Align {
  if (axis == Axis::Horizontal) {
    return m_content_halign.value_or(m_child_slot.style.horizontal_alignment);
  }
  return m_content_valign.value_or(m_child_slot.style.vertical_alignment);
}

} // namespace zod::sodium
