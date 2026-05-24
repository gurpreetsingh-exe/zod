#pragma once

#include "sodium/style.hh"
#include "sodium/widgets/widget.hh"

namespace zod::sodium {

struct SingleChildSlot {
  SharedPtr<Widget> child;
  SlotStyle style {};
};

class CompoundWidget : public Widget {
public:
  CompoundWidget() = default;

  auto set_child(SharedPtr<Widget> child, const SlotStyle& style = {})
      -> SingleChildSlot&;

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;
  auto get_children() const -> WidgetChildren override {
    if (m_child_slot.child) {
      return { m_child_slot.child };
    }
    return {};
  }

protected:
  SingleChildSlot m_child_slot;
};

} // namespace zod::sodium
