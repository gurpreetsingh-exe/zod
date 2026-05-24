#pragma once

#include "sodium/widgets/container.hh"

namespace zod ::sodium {

class Overlay : public Container {
public:
  Overlay() = default;

  struct OverlaySlot {
    SharedPtr<Widget> child;
    SlotStyle style {};
    int layer = 0;
  };

  auto add_overlay_child(SharedPtr<Widget> child, const SlotStyle& style = {},
                         int layer = 0) -> OverlaySlot&;

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;
  auto get_children() const -> WidgetChildren override;

private:
  Vector<OverlaySlot> m_overlay_children;
};

} // namespace zod::sodium
