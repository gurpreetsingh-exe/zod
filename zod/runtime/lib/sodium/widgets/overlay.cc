#include "sodium/widgets/overlay.hh"

namespace zod::sodium {

auto Overlay::add_overlay_child(SharedPtr<Widget> child, const SlotStyle& style,
                                int layer) -> Overlay::OverlaySlot& {
  m_overlay_children.push_back({ child, style, layer });
  return m_overlay_children.back();
}

auto Overlay::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }

  auto padding = m_style.padding.combined();
  auto horizontal_size = padding.x;
  auto vertical_size = padding.y;
  auto inner = Rect { {}, available }.padding(m_style.padding);

  for (const auto& slot : m_overlay_children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }

    auto slot_inner = inner.padding(slot.style.padding);
    auto desired = slot.child->compute_desired_size(slot_inner.size);
    auto slot_w = overlay_width(slot.style, desired, slot_inner.size.x, false);
    auto slot_h = overlay_height(slot.style, desired, slot_inner.size.y, false);
    auto slot_padding = slot.style.padding.combined();
    horizontal_size =
        std::max(horizontal_size, slot_w + padding.x + slot_padding.x);
    vertical_size =
        std::max(vertical_size, slot_h + padding.y + slot_padding.y);
  }

  return cache_desired_size({ horizontal_size, vertical_size });
}

auto Overlay::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  auto inner = m_frame.padding(m_style.padding);

  for (auto& slot : m_overlay_children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }

    auto slot_inner = inner.padding(slot.style.padding);
    auto desired = slot.child->desired_size();
    auto horizontal_size =
        overlay_width(slot.style, desired, slot_inner.size.x, true);
    auto vertical_size =
        overlay_height(slot.style, desired, slot_inner.size.y, true);
    auto aligned = align_within(slot_inner, { horizontal_size, vertical_size },
                                slot.style.horizontal_alignment,
                                slot.style.vertical_alignment);
    slot.child->arrange(aligned);
  }
}

auto Overlay::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }

  push_self_draws(cx);
  auto slots = Vector<const OverlaySlot*> {};
  slots.reserve(m_overlay_children.size());
  for (const auto& slot : m_overlay_children) { slots.push_back(&slot); }
  std::stable_sort(slots.begin(), slots.end(),
                   [](const OverlaySlot* a, const OverlaySlot* b) {
                     return a->layer < b->layer;
                   });

  for (const auto* slot : slots) {
    if (slot->child) {
      slot->child->paint(cx);
    }
  }
}

auto Overlay::get_children() const -> WidgetChildren {
  auto children = Container::get_children();
  children.reserve(children.size() + m_overlay_children.size());
  for (const auto& slot : m_overlay_children) {
    children.push_back(slot.child);
  }
  return children;
}

} // namespace zod::sodium
