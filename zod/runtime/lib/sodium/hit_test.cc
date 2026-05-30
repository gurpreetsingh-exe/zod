#include "sodium/hit_test.hh"

namespace zod::sodium {

auto HitTestList::clear() -> void {
  m_entries.clear();
  m_next_order = 0;
}

auto HitTestList::build_from(Widget& root) -> void {
  clear();
  collect(root, {}, 0);
}

auto HitTestList::collect(Widget& widget, WidgetPath path, i32 layer) -> void {
  auto visibility = widget.visibility();
  if (visibility == Visibility::Hidden or visibility == Visibility::Collapsed) {
    return;
  }

  path.push(&widget);
  auto entry_layer = layer + widget.hit_test_layer();
  if (visibility != Visibility::SelfHitTestInvisible) {
    m_entries.push_back({ path, widget.hit_test_bounds(), entry_layer,
                          widget.hit_test_priority(), m_next_order++ });
  }

  for (auto child : widget.get_children()) {
    if (child) {
      collect(*child, path, entry_layer);
    }
  }
}

auto HitTestList::path_at(vec2 mouse) const -> WidgetPath {
  const auto* best = static_cast<const HitTestEntry*>(nullptr);
  for (const auto& entry : m_entries) {
    if (not entry.bounds.intersect(mouse)) {
      continue;
    }

    if (not best) {
      best = &entry;
      continue;
    }

    if (entry.layer != best->layer) {
      if (entry.layer > best->layer) {
        best = &entry;
      }
      continue;
    }

    if (entry.priority != best->priority) {
      if (entry.priority > best->priority) {
        best = &entry;
      }
      continue;
    }

    if (entry.order != best->order) {
      if (entry.order > best->order) {
        best = &entry;
      }
      continue;
    }

    if (entry.path.size() > best->path.size()) {
      best = &entry;
    }
  }

  return best ? best->path : WidgetPath {};
}

} // namespace zod::sodium
