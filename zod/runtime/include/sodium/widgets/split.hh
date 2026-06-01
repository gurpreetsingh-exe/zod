#pragma once

#include "sodium/gui.hh"
#include "sodium/widgets/box_container.hh"
#include "sodium/widgets/compound_widget.hh"

namespace zod::sodium {

class Splitter : public BoxContainer {
public:
  struct Slot {
    SharedPtr<Widget> child;
    f32 size_coefficient = 1.0f;
    f32 min_size = 20.0f;
    bool resizable = true;
  };

  Splitter(Axis = Axis::Horizontal, f32 = S.padding, WidgetStyle = {});

  auto add_slot(SharedPtr<Widget>, f32 = 1.0f, f32 = 20.0f, bool = true)
      -> usize;

  auto set_axis(Axis) -> void;
  auto axis() const -> Axis { return m_axis; }

protected:
  auto rebuild_children() -> void;
  auto content_slot(f32) const -> SlotStyle;
  auto handle_slot() const -> SlotStyle;
  auto set_slot_weight(usize, f32) -> void;

private:
  struct ResizeState {
    bool resizing = false;
    usize handle = 0;
    vec2 start_mouse = {};
    f32 previous_start_size = 0.0f;
    f32 next_start_size = 0.0f;
    f32 previous_start_weight = 1.0f;
    f32 next_start_weight = 1.0f;
  };

  auto insert_slot(usize, SharedPtr<Widget>, f32 = 1.0f, f32 = 20.0f,
                   bool = true) -> usize;
  static auto slot_child_index(usize) -> usize;
  auto create_handle(usize) -> SharedPtr<Widget>;
  auto handle_hit_test_margin() const -> Padding;
  auto cursor() const -> cursor_shape_t;
  auto begin_resize(usize, const Event&) -> EventResponse;
  auto update_resize(const Event&) -> EventResponse;
  auto end_resize(const Event&) -> EventResponse;
  auto can_resize_handle(usize) const -> bool;

private:
  Axis m_axis = Axis::Horizontal;
  f32 m_splitter_thickness = S.padding;
  WidgetStyle m_separator_style {};
  Vector<Slot> m_slots;
  ResizeState m_resize_state = {};
};

class Split : public Splitter {
public:
  Split(SharedPtr<Widget>, SharedPtr<Widget>, Axis = Axis::Horizontal,
        f32 = 0.5f, f32 = S.padding, WidgetStyle = {});

  Split(SharedPtr<Widget> first, SharedPtr<Widget> second, f32 split)
      : Split(first, second, Axis::Horizontal, split) {}
};

} // namespace zod::sodium
