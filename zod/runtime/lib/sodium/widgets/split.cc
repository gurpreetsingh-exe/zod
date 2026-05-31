#include "sodium/widgets/split.hh"
#include "sodium/gui.hh"
#include "sodium/widgets/builder.hh"

namespace zod::sodium {

Split::Split(SharedPtr<Widget> first_, SharedPtr<Widget> second_, Axis axis,
             f32 s)
    : BoxContainer(axis), m_axis(axis), state({ s }) {
  add_child(first_, split_slot(state.split));
  add_child(*create<Box>()
                 .hit_test_margin(get_hit_test_margin())
                 .hit_test_priority(10)
                 .cursor(cursor())
                 .on_mouse_down(this, &Split::begin_resize)
                 .on_mouse_move(this, &Split::update_resize)
                 .on_mouse_up(this, &Split::end_resize),
            handle_slot());
  add_child(second_, split_slot(1.0f - state.split));
}

auto Split::split_slot(f32 weight) const -> SlotStyle {
  if (m_axis == Axis::Horizontal) {
    return slot().fill_width(weight);
  }
  return slot().fill_height(weight);
}

auto Split::handle_slot() const -> SlotStyle {
  if (m_axis == Axis::Horizontal) {
    return slot().fixed_width(S.padding);
  }
  return slot().fixed_height(S.padding);
}

auto Split::get_hit_test_margin() const -> Padding {
  return m_axis == Axis::Horizontal ? Padding { 8, 8, 0, 0 }
                                    : Padding { 0, 0, 8, 8 };
}

auto Split::cursor() const -> cursor_shape_t {
  if (m_axis == Axis::Horizontal) {
    return cursor_shape_t::ResizeHorizontal;
  }
  return cursor_shape_t::ResizeVertical;
}

auto Split::begin_resize(const Event& event) -> EventResponse {
  if (event.button != MouseButton::Left) {
    return EventResponse::unhandled();
  }
  state.resizing = true;
  state.start_mouse = event.mouse;
  state.start_split = state.split;
  return EventResponse::handled().capture_mouse(MouseButton::Left);
}

auto Split::update_resize(const Event& event) -> EventResponse {
  if (not state.resizing) {
    return EventResponse::unhandled();
  }

  auto resizable_size = std::max(1.0f, frame().size[m_axis] - S.padding);
  auto delta = event.mouse[m_axis] - state.start_mouse[m_axis];
  auto fac = state.start_split + delta / resizable_size;

  state.split = glm::clamp(fac, 0.05f, 0.95f);
  set_split_weight(get_slot(0).style, state.split);
  set_split_weight(get_slot(2).style, 1.0f - state.split);
  invalidate_layout();

  return EventResponse::handled();
}

auto Split::end_resize(const Event& event) -> EventResponse {
  if (event.button == MouseButton::Left and state.resizing) {
    state.resizing = false;
    return EventResponse::handled();
  }
  return EventResponse::unhandled();
}

auto Split::set_split_weight(SlotStyle& style, f32 weight) const -> void {
  if (m_axis == Axis::Horizontal) {
    style.horizontal_stretch_weight = weight;
  } else {
    style.vertical_stretch_weight = weight;
  }
}

} // namespace zod::sodium
