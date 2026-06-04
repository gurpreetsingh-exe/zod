#pragma once

#include "sodium/widgets/box.hh"
#include "sodium/widgets/box_container.hh"
#include "sodium/widgets/compound_widget.hh"

namespace zod::sodium {

template <class ItemT>
class ListView : public CompoundWidget {
public:
  using ItemsSource = std::function<Vector<ItemT>()>;
  using GenerateRow = std::function<SharedPtr<Widget>(const ItemT&, usize)>;
  using UpdateRow =
      std::function<void(const SharedPtr<Widget>&, const ItemT&, usize)>;

  auto set_items_source(ItemsSource source) -> void {
    m_items_source = std::move(source);
    request_refresh();
  }

  auto set_on_generate_row(GenerateRow generator) -> void {
    m_on_generate_row = std::move(generator);
    m_needs_rebuild = true;
    request_refresh();
  }

  auto set_on_update_row(UpdateRow updater) -> void {
    m_on_update_row = std::move(updater);
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
    if (not m_needs_refresh) {
      return;
    }

    if (not m_rows_widget or m_needs_rebuild) {
      rebuild_rows();
    }

    auto items = m_items_source ? m_items_source() : Vector<ItemT>();
    if (items.size() < m_rows.size()) {
      m_rows_widget->remove_children_from(items.size());
      m_rows.resize(items.size());
    }

    for (auto i = usize(0); i < items.size(); ++i) {
      if (i >= m_rows.size()) {
        auto widget = m_on_generate_row ? m_on_generate_row(items[i], i)
                                        : SharedPtr<Widget>(shared<Box>());
        m_rows.push_back({ widget });
        m_rows_widget->add_child(widget, slot().fixed_height(m_row_height));
      }

      m_rows_widget->get_slot(i).style = slot().fixed_height(m_row_height);
      if (m_on_update_row) {
        m_on_update_row(m_rows[i].widget, items[i], i);
      }
    }

    m_needs_refresh = false;
    invalidate_layout();
  }

  auto rebuild_rows() -> void {
    m_rows.clear();
    m_rows_widget = shared<VerticalBox>();
    set_child(m_rows_widget);
    m_needs_rebuild = false;
  }

private:
  struct Row {
    SharedPtr<Widget> widget;
  };

private:
  SharedPtr<VerticalBox> m_rows_widget;
  Vector<Row> m_rows;
  ItemsSource m_items_source;
  GenerateRow m_on_generate_row;
  UpdateRow m_on_update_row;
  f32 m_row_height = 24.0f;
  bool m_needs_rebuild = true;
  bool m_needs_refresh = true;
};

} // namespace zod::sodium
