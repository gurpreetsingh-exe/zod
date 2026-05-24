#pragma once

#include "sodium/style.hh"
#include "sodium/widgets/widget.hh"

namespace zod::sodium {

class Container : public Widget {
public:
  Container() = default;

  struct Slot {
    SharedPtr<Widget> child;
    SlotStyle style {};
  };

  auto add_child(SharedPtr<Widget> child, const SlotStyle& style = {}) -> Slot&;
  auto child_count() const -> usize { return m_children.size(); }
  auto get_children() const -> WidgetChildren override;

protected:
  Vector<Slot> m_children;
};

} // namespace zod::sodium
