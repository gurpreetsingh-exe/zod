#include "sodium_v2/widgets/split.hh"
#include "sodium_v2/widgets/builder.hh"

namespace zod::sodium {

Splitter::Splitter(Axis axis, f32 splitter_thickness,
                   WidgetStyle separator_style)
    : BoxContainer(axis), m_axis(axis),
      m_splitter_thickness(splitter_thickness),
      m_separator_style(separator_style) {}

auto Splitter::add_slot(SharedPtr<Widget> child, f32 size_coefficient,
                        f32 min_size, bool resizable) -> usize {
  return insert_slot(m_slots.size(), child, size_coefficient, min_size,
                     resizable);
}

auto Splitter::insert_slot(usize index, SharedPtr<Widget> child,
                           f32 size_coefficient, f32 min_size, bool resizable)
    -> usize {
  ZASSERT(child);
  index = std::min(index, m_slots.size());
  auto slot = Slot { child, std::max(0.0f, size_coefficient),
                     std::max(0.0f, min_size), resizable };
  m_slots.insert(m_slots.begin() + index, slot);
  rebuild_children();
  return index;
}

auto Splitter::set_axis(Axis axis) -> void {
  m_axis = axis;
  BoxContainer::set_axis(axis);
  rebuild_children();
}

auto Splitter::rebuild_children() -> void {
  m_children.clear();
  for (auto index = usize(0); index < m_slots.size(); ++index) {
    add_child(m_slots[index].child,
              content_slot(m_slots[index].size_coefficient));
    if (index + 1 < m_slots.size()) {
      add_child(create_handle(index), handle_slot());
    }
  }
  invalidate_layout();
}

auto Splitter::content_slot(f32 weight) const -> SlotStyle {
  if (m_axis == Axis::Horizontal) {
    return slot().fill_width(weight);
  }
  return slot().fill_height(weight);
}

auto Splitter::handle_slot() const -> SlotStyle {
  if (m_axis == Axis::Horizontal) {
    return slot().fixed_width(m_splitter_thickness);
  }
  return slot().fixed_height(m_splitter_thickness);
}

auto Splitter::set_slot_weight(usize slot_index, f32 weight) -> void {
  ZASSERT(slot_index < m_slots.size());
  m_slots[slot_index].size_coefficient = std::max(0.0f, weight);

  auto child_index = slot_child_index(slot_index);
  ZASSERT(child_index < m_children.size());
  if (m_axis == Axis::Horizontal) {
    m_children[child_index].style.horizontal_stretch_weight =
        m_slots[slot_index].size_coefficient;
  } else {
    m_children[child_index].style.vertical_stretch_weight =
        m_slots[slot_index].size_coefficient;
  }
}

auto Splitter::slot_child_index(usize slot_index) -> usize {
  return slot_index * 2;
}

auto Splitter::create_handle(usize handle_index) -> SharedPtr<Widget> {
  auto inner = *create<Box>();
  inner->style() = m_separator_style;

  return *create<CompoundWidget>()
              .hit_test_margin(handle_hit_test_margin())
              .hit_test_priority(10)
              .cursor(cursor())
              .on_mouse_down([this, handle_index](const Event& event) {
                return begin_resize(handle_index, event);
              })
              .on_mouse_move(this, &Splitter::update_resize)
              .on_mouse_up(this, &Splitter::end_resize)
              .child(inner);
}

auto Splitter::handle_hit_test_margin() const -> Padding {
  return m_axis == Axis::Horizontal ? Padding { 8, 8, 0, 0 }
                                    : Padding { 0, 0, 8, 8 };
}

auto Splitter::cursor() const -> cursor_shape_t {
  if (m_axis == Axis::Horizontal) {
    return cursor_shape_t::ResizeHorizontal;
  }
  return cursor_shape_t::ResizeVertical;
}

auto Splitter::begin_resize(usize handle_index, const Event& event)
    -> EventResponse {
  if (event.button != MouseButton::Left or
      not can_resize_handle(handle_index)) {
    return EventResponse::unhandled();
  }

  auto previous = handle_index;
  auto next = handle_index + 1;

  m_resize_state.resizing = true;
  m_resize_state.handle = handle_index;
  m_resize_state.start_mouse = event.mouse;
  m_resize_state.previous_start_size =
      m_slots[previous].child->frame().size[m_axis];
  m_resize_state.next_start_size = m_slots[next].child->frame().size[m_axis];
  m_resize_state.previous_start_weight = m_slots[previous].size_coefficient;
  m_resize_state.next_start_weight = m_slots[next].size_coefficient;

  return EventResponse::handled().capture_mouse(MouseButton::Left);
}

auto Splitter::update_resize(const Event& event) -> EventResponse {
  if (not m_resize_state.resizing) {
    return EventResponse::unhandled();
  }

  auto previous = m_resize_state.handle;
  auto next = m_resize_state.handle + 1;
  if (not can_resize_handle(m_resize_state.handle)) {
    return EventResponse::unhandled();
  }

  auto delta = event.mouse[m_axis] - m_resize_state.start_mouse[m_axis];
  auto previous_min = m_slots[previous].min_size;
  auto next_min = m_slots[next].min_size;
  auto min_delta = previous_min - m_resize_state.previous_start_size;
  auto max_delta = m_resize_state.next_start_size - next_min;
  delta = std::clamp(delta, min_delta, max_delta);

  auto previous_size = m_resize_state.previous_start_size + delta;
  auto next_size = m_resize_state.next_start_size - delta;
  auto pair_size = std::max(1.0f, previous_size + next_size);
  auto pair_weight = std::max(0.0f, m_resize_state.previous_start_weight +
                                        m_resize_state.next_start_weight);

  if (pair_weight <= 0.0f) {
    pair_weight = 1.0f;
  }

  auto previous_weight = pair_weight * (previous_size / pair_size);
  auto next_weight = pair_weight - previous_weight;

  set_slot_weight(previous, previous_weight);
  set_slot_weight(next, next_weight);
  invalidate_layout();

  return EventResponse::handled();
}

auto Splitter::end_resize(const Event& event) -> EventResponse {
  if (event.button == MouseButton::Left and m_resize_state.resizing) {
    m_resize_state.resizing = false;
    return EventResponse::handled();
  }
  return EventResponse::unhandled();
}

auto Splitter::can_resize_handle(usize handle_index) const -> bool {
  return handle_index + 1 < m_slots.size() and
         m_slots[handle_index].resizable and
         m_slots[handle_index + 1].resizable;
}

Split::Split(SharedPtr<Widget> first, SharedPtr<Widget> second, Axis axis,
             f32 split, f32 splitter_thickness, WidgetStyle separator_style)
    : Splitter(axis, splitter_thickness, separator_style) {
  split = std::clamp(split, 0.0f, 1.0f);
  add_slot(first, split);
  add_slot(second, 1.0f - split);
}

} // namespace zod::sodium
