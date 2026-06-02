#include "sodium/hit_test.hh"

namespace zod::sodium {

auto HitTestList::clear() -> void {
  m_entries.clear();
  m_next_order = 0;
}

auto HitTestList::build_from(SharedPtr<Widget> root) -> void {
  ZASSERT(root);
  clear();
  collect(root, {}, 0);
}

auto HitTestList::collect(SharedPtr<Widget> widget, WidgetPath path, i32 layer)
    -> void {
  ZASSERT(widget);

  auto visibility = widget->visibility();
  if (visibility == Visibility::Hidden or visibility == Visibility::Collapsed) {
    return;
  }

  path.push(widget);
  auto entry_layer = layer + widget->hit_test_layer();
  if (visibility != Visibility::SelfHitTestInvisible) {
    m_entries.push_back({ path, widget->hit_test_bounds(), entry_layer,
                          widget->hit_test_priority(), m_next_order++ });
  }

  for (auto child : widget->get_children()) {
    collect(child, path, entry_layer);
  }
}

auto HitTestList::path_at(vec2 mouse) const -> WidgetPath {
  auto has_best = false;
  auto best = HitTestEntry {};
  for (const auto& entry : m_entries) {
    if (not entry.bounds.intersect(mouse)) {
      continue;
    }

    if (not has_best) {
      best = entry;
      has_best = true;
      continue;
    }

    if (entry.layer != best.layer) {
      if (entry.layer > best.layer) {
        best = entry;
      }
      continue;
    }

    if (entry.priority != best.priority) {
      if (entry.priority > best.priority) {
        best = entry;
      }
      continue;
    }

    if (entry.order != best.order) {
      if (entry.order > best.order) {
        best = entry;
      }
      continue;
    }

    if (entry.path.size() > best.path.size()) {
      best = entry;
    }
  }

  return has_best ? best.path : WidgetPath {};
}

} // namespace zod::sodium
