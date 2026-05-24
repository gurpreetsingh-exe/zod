#include "sodium/layout.hh"

namespace zod::sodium {

auto axis_padding_start(const Padding& padding, Axis axis) -> f32 {
  if (axis == Axis::Horizontal) {
    return padding.left;
  }
  return padding.top;
}

auto axis_size_rule(const SlotStyle& style, Axis axis) -> SizeRule {
  if (axis == Axis::Horizontal) {
    return style.horizontal_rule;
  }
  return style.vertical_rule;
}

auto axis_alignment(const SlotStyle& style, Axis axis) -> Align {
  if (axis == Axis::Horizontal) {
    return style.horizontal_alignment;
  }
  return style.vertical_alignment;
}

auto axis_stretch_weight(const SlotStyle& style, Axis axis) -> f32 {
  if (axis == Axis::Horizontal) {
    return style.horizontal_stretch_weight;
  }
  return style.vertical_stretch_weight;
}

auto auto_or_fixed_size(SizeRule rule, f32 fixed, f32 desired) -> f32 {
  if (rule == SizeRule::Fixed) {
    return fixed;
  }
  if (rule == SizeRule::Auto) {
    return std::max(0.0f, desired);
  }
  return 0.0f;
}

auto axis_auto_or_fixed_size(const SlotStyle& style, const vec2& desired,
                             Axis axis) -> f32 {
  return auto_or_fixed_size(axis_size_rule(style, axis), style.size[axis],
                            desired[axis]);
}

auto slot_size(const SlotStyle& style, const vec2& desired) -> vec2 {
  return { axis_auto_or_fixed_size(style, desired, Axis::Horizontal),
           axis_auto_or_fixed_size(style, desired, Axis::Vertical) };
}

auto overlay_axis_size(const SlotStyle& style, const vec2& desired,
                       f32 available, bool allow_stretch, Axis axis) -> f32 {
  auto rule = axis_size_rule(style, axis);
  if (rule == SizeRule::Fixed) {
    return style.size[axis];
  }
  if (allow_stretch and rule == SizeRule::Stretch) {
    return available;
  }
  return std::max(0.0f, desired[axis]);
}

auto overlay_width(const SlotStyle& style, const vec2& desired, f32 available,
                   bool allow_stretch) -> f32 {
  return overlay_axis_size(style, desired, available, allow_stretch,
                           Axis::Horizontal);
}

auto overlay_height(const SlotStyle& style, const vec2& desired, f32 available,
                    bool allow_stretch) -> f32 {
  return overlay_axis_size(style, desired, available, allow_stretch,
                           Axis::Vertical);
}

auto align_axis(Rect& result, const Rect& outer, const vec2& desired, Axis axis,
                Align alignment) -> void {
  if (alignment == Align::Stretch) {
    result.size[axis] = outer.size[axis];
    return;
  }

  auto size = std::min(desired[axis], outer.size[axis]);
  result.size[axis] = size;

  auto free_space = outer.size[axis] - size;
  if (alignment == Align::Center) {
    result.position[axis] += free_space * 0.5f;
  } else if (alignment == Align::End) {
    result.position[axis] += free_space;
  }
}

auto align_within(const Rect& outer, const vec2& desired, Align horizontal,
                  Align vertical) -> Rect {
  auto result = outer;
  auto desired_size = glm::max(vec2 { 0.0f }, desired);

  align_axis(result, outer, desired_size, Axis::Horizontal, horizontal);
  align_axis(result, outer, desired_size, Axis::Vertical, vertical);

  return result;
}

} // namespace zod::sodium
