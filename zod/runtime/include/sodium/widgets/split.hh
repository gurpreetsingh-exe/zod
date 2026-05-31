#pragma once

#include "sodium/widgets/box_container.hh"
#include "sodium/widgets/compound_widget.hh"
#include "sodium/gui.hh"

namespace zod::sodium {

class Split : public BoxContainer {
  struct ResizeState {
    f32 split = 0.5f;
    bool resizing = false;
    vec2 start_mouse = {};
    f32 start_split = 0.5f;
  };

public:
  Split(SharedPtr<Widget>, SharedPtr<Widget>, Axis = Axis::Horizontal,
        f32 = 0.5f, f32 = S.padding, SharedPtr<Widget> = shared<CompoundWidget>());

  Split(SharedPtr<Widget> first_, SharedPtr<Widget> second_, f32 s)
      : Split(first_, second_, Axis::Horizontal, s) {}

private:
  auto split_slot(f32) const -> SlotStyle;
  auto handle_slot() const -> SlotStyle;
  auto get_hit_test_margin() const -> Padding;
  auto cursor() const -> cursor_shape_t;
  auto begin_resize(const Event&) -> EventResponse;
  auto update_resize(const Event&) -> EventResponse;
  auto end_resize(const Event&) -> EventResponse;
  auto set_split_weight(SlotStyle&, f32) const -> void;

private:
  Axis m_axis = Axis::Horizontal;
  f32 m_splitter_thickness = S.padding;
  ResizeState state = {};
};

} // namespace zod::sodium
