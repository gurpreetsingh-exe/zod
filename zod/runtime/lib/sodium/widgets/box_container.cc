#include "sodium/widgets/box_container.hh"

namespace zod::sodium {

auto measure_stack_children(const Vector<Container::Slot>& children,
                            const WidgetStyle& style, Axis axis, vec2 available)
    -> vec2 {
  auto opposite_axis = ~axis;
  auto used = vec2();
  auto visible = usize(0);

  for (const auto& slot : children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }

    auto slot_box = Rect { 0.0f, 0.0f, available.x, available.y };
    auto inner = slot_box.padding(slot.style.padding);
    auto desired = slot.child->compute_desired_size(inner.size);

    auto child_size =
        slot_size(slot.style, desired) + slot.style.padding.combined();

    used[axis] += child_size[axis];
    used[opposite_axis] =
        std::max(used[opposite_axis], child_size[opposite_axis]);
    ++visible;
  }

  auto gaps = 0.0f;
  if (visible > 1) {
    gaps = style.gap * f32(visible - 1);
  }

  used[axis] += gaps;
  return used + style.padding.combined();
}

auto BoxContainer::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  return cache_desired_size(
      measure_stack_children(m_children, m_style, m_axis, available));
}

auto BoxContainer::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  auto inner = m_frame.padding(m_style.padding);
  auto layout_axis = m_axis;
  auto opposite_axis = ~layout_axis;

  auto fixed_layout_size = 0.0f;
  auto stretch_weight_sum = 0.0f;
  auto visible = usize(0);

  for (const auto& slot : m_children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }
    ++visible;
    auto layout_rule = axis_size_rule(slot.style, layout_axis);
    auto layout_padding = slot.style.padding.combined()[layout_axis];

    fixed_layout_size += layout_padding;
    if (layout_rule == SizeRule::Stretch) {
      stretch_weight_sum +=
          std::max(0.0f, axis_stretch_weight(slot.style, layout_axis));
    } else if (layout_rule == SizeRule::Fixed) {
      fixed_layout_size += slot.style.size[layout_axis];
    } else {
      auto desired = slot.child->desired_size();
      auto desired_layout_size = desired[layout_axis];
      if (desired_layout_size > 0.0f) {
        fixed_layout_size += desired_layout_size;
      }
    }
  }

  auto gaps = 0.0f;
  if (visible > 1) {
    gaps = m_style.gap * f32(visible - 1);
  }

  auto available_layout_size = inner.size[layout_axis];
  auto available_opposite_size = inner.size[opposite_axis];
  auto cursor = inner.position[layout_axis];

  auto remaining_layout_size =
      std::max(0.0f, available_layout_size - fixed_layout_size - gaps);

  for (auto& slot : m_children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }
    auto child_desired = slot.child->desired_size();
    auto layout_rule = axis_size_rule(slot.style, layout_axis);
    auto opposite_rule = axis_size_rule(slot.style, opposite_axis);
    auto fixed_layout_size = slot.style.size[layout_axis];
    auto fixed_opposite_size = slot.style.size[opposite_axis];
    auto desired_layout_size = child_desired[layout_axis];
    auto desired_opposite_size = child_desired[opposite_axis];
    auto opposite_alignment = axis_alignment(slot.style, opposite_axis);
    auto layout_padding = slot.style.padding.combined()[layout_axis];
    auto opposite_padding = slot.style.padding.combined()[opposite_axis];
    auto slot_opposite_size =
        std::max(0.0f, available_opposite_size - opposite_padding);

    auto layout_size = 0.0f;
    if (layout_rule == SizeRule::Fixed) {
      layout_size = fixed_layout_size;
    } else if (layout_rule == SizeRule::Stretch) {
      auto weight = 1.0f / std::max(1ul, visible);
      if (stretch_weight_sum > 0.0f) {
        weight = std::max(0.0f, axis_stretch_weight(slot.style, layout_axis)) /
                 stretch_weight_sum;
      }
      layout_size = remaining_layout_size * weight;
    } else {
      layout_size = std::max(0.0f, desired_layout_size);
    }

    auto opposite_size = 0.0f;
    if (opposite_rule == SizeRule::Fixed) {
      opposite_size = fixed_opposite_size;
    } else if (opposite_rule == SizeRule::Stretch) {
      opposite_size = slot_opposite_size;
    } else if (opposite_alignment == Align::Stretch) {
      opposite_size = slot_opposite_size;
    } else {
      opposite_size = std::max(0.0f, desired_opposite_size);
    }

    auto child_outer = Rect {};
    child_outer.position[layout_axis] =
        cursor + axis_padding_start(slot.style.padding, layout_axis);
    child_outer.position[opposite_axis] =
        inner.position[opposite_axis] +
        axis_padding_start(slot.style.padding, opposite_axis);
    child_outer.size[layout_axis] = layout_size;
    child_outer.size[opposite_axis] = opposite_size;

    auto aligned = align_within(child_outer, child_desired,
                                slot.style.horizontal_alignment,
                                slot.style.vertical_alignment);
    slot.child->arrange(aligned);
    cursor += layout_size + layout_padding + m_style.gap;
  }
}

auto BoxContainer::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }
  push_self_draws(cx);
  for (const auto& slot : m_children) {
    if (slot.child) {
      slot.child->paint(cx);
    }
  }
}

} // namespace zod::sodium
