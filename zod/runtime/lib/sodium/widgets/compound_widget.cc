#include "sodium/widgets/compound_widget.hh"

namespace zod::sodium {

auto CompoundWidget::set_child(SharedPtr<Widget> child, const SlotStyle& style)
    -> SingleChildSlot& {
  m_child_slot = { child, style };
  return m_child_slot;
}

auto CompoundWidget::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  if (not m_child_slot.child) {
    return cache_desired_size(m_style.padding.combined());
  }

  auto inner = Rect { {}, available }.padding(m_style.padding);
  auto child_inner = inner.padding(m_child_slot.style.padding);
  auto desired = m_child_slot.child->compute_desired_size(child_inner.size);

  return cache_desired_size(desired + m_style.padding.combined() +
                            m_child_slot.style.padding.combined());
}

auto CompoundWidget::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  if (not m_child_slot.child) {
    return;
  }

  auto child_outer =
      m_frame.padding(m_style.padding).padding(m_child_slot.style.padding);
  auto desired = m_child_slot.child->desired_size();
  auto aligned = align_within(child_outer, desired,
                              m_child_slot.style.horizontal_alignment,
                              m_child_slot.style.vertical_alignment);
  m_child_slot.child->arrange(aligned);
}

auto CompoundWidget::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }
  push_self_draws(cx);
  if (m_child_slot.child) {
    m_child_slot.child->paint(cx);
  }
}

} // namespace zod::sodium
