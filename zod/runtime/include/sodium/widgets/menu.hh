#pragma once

#include "sodium/widgets/widget.hh"

namespace zod::sodium {

class Menu : public Widget {
public:
  Menu() = default;

  auto compute_desired_size(vec2) -> vec2 override;
  auto paint(PaintCx&) const -> void override;
  auto arrange(const Rect&) -> void override;
  auto get_children() const -> WidgetChildren override;

private:
  bool m_open = false;
  Vector<SharedPtr<Widget>> m_buttons;
};

} // namespace zod::sodium
