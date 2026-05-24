#pragma once

#include "core/shapes.hh"
#include "sodium/style.hh"

namespace zod::sodium {

auto axis_padding_start(const Padding&, Axis) -> f32;
auto axis_size_rule(const SlotStyle&, Axis) -> SizeRule;
auto axis_alignment(const SlotStyle&, Axis) -> Align;
auto axis_stretch_weight(const SlotStyle&, Axis) -> f32;
auto auto_or_fixed_size(SizeRule, f32, f32) -> f32;
auto axis_auto_or_fixed_size(const SlotStyle&, const vec2&, Axis) -> f32;
auto slot_size(const SlotStyle&, const vec2&) -> vec2;
auto overlay_axis_size(const SlotStyle&, const vec2&, f32, bool, Axis) -> f32;
auto overlay_width(const SlotStyle&, const vec2&, f32, bool) -> f32;
auto overlay_height(const SlotStyle&, const vec2&, f32, bool) -> f32;
auto align_axis(Rect&, const Rect&, const vec2&, Axis, Align) -> void;
auto align_within(const Rect&, const vec2&, Align, Align) -> Rect;

} // namespace zod::sodium
