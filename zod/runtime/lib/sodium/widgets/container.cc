#include "sodium/widgets/container.hh"

namespace zod::sodium {

auto Container::add_child(SharedPtr<Widget> child, const SlotStyle& style)
    -> Container::Slot& {
  m_children.push_back({ child, style });
  return m_children.back();
}

auto Container::get_children() const -> WidgetChildren {
  auto children = WidgetChildren {};
  children.reserve(m_children.size());
  for (const auto& slot : m_children) { children.push_back(slot.child); }
  return children;
}

} // namespace zod::sodium
