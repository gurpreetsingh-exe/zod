#pragma once

#include "sodium_v2/style.hh"
#include "sodium_v2/widgets/widget.hh"

namespace zod::sodium {

class Container : public Widget {
public:
  Container() = default;

  struct Slot {
    SharedPtr<Widget> child;
    SlotStyle style {};
  };

  auto add_child(SharedPtr<Widget> child, const SlotStyle& style = {}) -> Slot&;
  auto clear_children() -> void;
  auto remove_children_from(usize index) -> void;
  auto child_count() const -> usize { return m_children.size(); }
  auto get_children() const -> WidgetChildren override;

protected:
  Vector<Slot> m_children;
};

} // namespace zod::sodium
