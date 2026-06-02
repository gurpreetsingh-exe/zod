#pragma once

#include "sodium/widgets/box_container.hh"
#include "sodium/widgets/compound_widget.hh"

namespace zod::sodium {

template <class ItemT>
class ListView : public CompoundWidget {
public:
  using ItemsSource = std::function<Vector<ItemT>()>;
  using GenerateRow = std::function<SharedPtr<Widget>(const ItemT&, usize)>;

  auto set_items_source(ItemsSource source) -> void {
    m_items_source = std::move(source);
    request_refresh();
  }

  auto set_on_generate_row(GenerateRow generator) -> void {
    m_on_generate_row = std::move(generator);
    request_refresh();
  }

  auto set_row_height(f32 row_height) -> void {
    m_row_height = row_height;
    request_refresh();
  }

  auto request_refresh() -> void {
    m_needs_refresh = true;
    invalidate_layout();
  }

  auto compute_desired_size(vec2 available) -> vec2 override {
    refresh();
    return CompoundWidget::compute_desired_size(available);
  }

private:
  auto refresh() -> void {
    // FIXME: refersh happens every frame so this is a bit sus
    if (not m_needs_refresh) {
      return;
    }

    auto rows = shared<VerticalBox>();
    if (m_items_source and m_on_generate_row) {
      auto items = m_items_source();
      for (auto i = usize(0); i < items.size(); ++i) {
        rows->add_child(m_on_generate_row(items[i], i),
                        slot().fixed_height(m_row_height));
      }
    }

    set_child(rows);
    m_needs_refresh = false;
    invalidate_layout();
  }

private:
  ItemsSource m_items_source;
  GenerateRow m_on_generate_row;
  f32 m_row_height = 24.0f;
  bool m_needs_refresh = true;
};

} // namespace zod::sodium
