#pragma once

#include "sodium/widgets/container.hh"

namespace zod::sodium {

class BoxContainer : public Container {
public:
  explicit BoxContainer(Axis axis = Axis::Vertical) : m_axis(axis) {}

  auto set_axis(Axis axis) -> void { m_axis = axis; }
  auto axis() const -> Axis { return m_axis; }
  auto get_slot(usize idx) -> Slot& { return m_children[idx]; }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;

private:
  Axis m_axis = Axis::Vertical;
};

class Stack : public BoxContainer {
public:
  explicit Stack(Axis axis = Axis::Vertical) : BoxContainer(axis) {}
};

class HorizontalBox : public BoxContainer {
public:
  HorizontalBox() : BoxContainer(Axis::Horizontal) {}

  static auto slot() -> SlotBuilder;
};

class VerticalBox : public BoxContainer {
public:
  VerticalBox() : BoxContainer(Axis::Vertical) {}

  static auto slot() -> SlotBuilder;
};

} // namespace zod::sodium
