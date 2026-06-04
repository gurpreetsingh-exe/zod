#pragma once

#include "sodium_v2/widgets/box.hh"
#include "sodium_v2/widgets/box_container.hh"
#include "sodium_v2/widgets/builder.hh"
#include "sodium_v2/widgets/compound_widget.hh"
#include "sodium_v2/widgets/split.hh"
#include "sodium_v2/widgets/text.hh"

namespace zod::sodium {

template <class ItemT>
class MultiColumnListView : public CompoundWidget {
public:
  using ItemsSource = std::function<Vector<ItemT>()>;
  using GenerateCell = std::function<SharedPtr<Widget>(const ItemT&, usize)>;
  using UpdateCell =
      std::function<void(const SharedPtr<Widget>&, const ItemT&, usize)>;
  using RowBackground = std::function<vec4(usize)>;

  struct Column {
    String name;
    f32 weight = 1.0f;
    GenerateCell on_generate_cell;
    UpdateCell on_update_cell;
    SharedPtr<VerticalBox> widget = nullptr;
  };

  auto add_column(String name, GenerateCell generator, f32 weight = 1.0f)
      -> void {
    m_columns.push_back({ .name = std::move(name),
                          .weight = weight,
                          .on_generate_cell = std::move(generator) });
    m_needs_rebuild = true;
    request_refresh();
  }

  auto add_column(String name, GenerateCell generator, UpdateCell updater,
                  f32 weight = 1.0f) -> void {
    m_columns.push_back({ .name = std::move(name),
                          .weight = weight,
                          .on_generate_cell = std::move(generator),
                          .on_update_cell = std::move(updater) });
    m_needs_rebuild = true;
    request_refresh();
  }

  auto set_items_source(ItemsSource source) -> void {
    m_items_source = std::move(source);
    request_refresh();
  }

  auto set_row_background(RowBackground background) -> void {
    m_row_background = std::move(background);
    request_refresh();
  }

  auto set_header_background(vec4 color) -> void {
    m_header_background = color;
    m_needs_rebuild = true;
    request_refresh();
  }

  auto set_header_height(f32 height) -> void {
    m_header_height = height;
    request_refresh();
  }

  auto set_row_height(f32 height) -> void {
    m_row_height = height;
    request_refresh();
  }

  auto set_splitter_thickness(f32 thickness) -> void {
    m_splitter_thickness = thickness;
    m_needs_rebuild = true;
    request_refresh();
  }

  auto set_splitter_style(WidgetStyle style) -> void {
    m_splitter_style = style;
    m_needs_rebuild = true;
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
    if (m_needs_rebuild) {
      rebuild_columns();
    }
    if (not m_needs_refresh) {
      return;
    }

    for (auto& column : m_columns) {
      column.widget->get_slot(0).style = slot().fixed_height(m_header_height);
    }

    auto items = m_items_source ? m_items_source() : Vector<ItemT>();
    if (items.size() < m_rows.size()) {
      for (auto& column : m_columns) {
        column.widget->remove_children_from(items.size() + 1);
      }
      m_rows.resize(items.size());
    }

    for (auto row = usize(0); row < items.size(); ++row) {
      if (row >= m_rows.size()) {
        add_row(items[row], row);
      }

      for (auto column_index = usize(0); column_index < m_columns.size();
           ++column_index) {
        auto& column = m_columns[column_index];
        column.widget->get_slot(row + 1).style =
            slot().fixed_height(m_row_height);
        m_rows[row].cell_boxes[column_index]->style().background =
            row_background(row);
        if (column.on_update_cell) {
          column.on_update_cell(m_rows[row].cells[column_index], items[row],
                                row);
        }
      }
    }

    m_needs_refresh = false;
    invalidate_layout();
  }

  auto rebuild_columns() -> void {
    m_rows.clear();
    for (auto& column : m_columns) {
      column.widget = shared<VerticalBox>();
      column.widget->add_child(header_cell(column.name),
                               slot().fixed_height(m_header_height));
    }

    if (m_columns.empty()) {
      set_child(shared<VerticalBox>());
    } else {
      set_child(split_from_column(0));
    }

    m_needs_rebuild = false;
  }

  auto add_row(const ItemT& item, usize row) -> void {
    auto row_data = Row();
    row_data.cells.reserve(m_columns.size());
    row_data.cell_boxes.reserve(m_columns.size());

    for (auto& column : m_columns) {
      auto cell = column.on_generate_cell ? column.on_generate_cell(item, row)
                                          : SharedPtr<Widget>(shared<Box>());
      auto row_box = create<Box>().background(row_background(row));
      row_box[cell];

      row_data.cells.push_back(cell);
      row_data.cell_boxes.push_back(*row_box);
      column.widget->add_child(*row_box, slot().fixed_height(m_row_height));
    }

    m_rows.push_back(std::move(row_data));
  }

  auto split_from_column(usize index) -> SharedPtr<Widget> {
    if (index + 1 >= m_columns.size()) {
      return m_columns[index].widget;
    }

    return shared<Split>(m_columns[index].widget, split_from_column(index + 1),
                         Axis::Horizontal, m_columns[index].weight,
                         m_splitter_thickness, m_splitter_style);
  }

  auto header_cell(const String& text) -> SharedPtr<Widget> {
    auto cell = create<Box>()
                    .background(m_header_background)
                    .content_valign(Align::Center);
    cell[create<Text>(text).padding(m_cell_padding)];
    return *cell;
  }

  auto row_background(usize row) const -> vec4 {
    if (m_row_background) {
      return m_row_background(row);
    }
    return {};
  }

private:
  struct Row {
    Vector<SharedPtr<Widget>> cells;
    Vector<SharedPtr<Box>> cell_boxes;
  };

private:
  Vector<Column> m_columns;
  Vector<Row> m_rows;
  ItemsSource m_items_source;
  RowBackground m_row_background;
  Padding m_cell_padding { 12.0f, 12.0f, 0.0f, 0.0f };
  vec4 m_header_background {};
  f32 m_header_height = 24.0f;
  f32 m_row_height = 24.0f;
  f32 m_splitter_thickness = S.padding;
  WidgetStyle m_splitter_style {};
  bool m_needs_rebuild = true;
  bool m_needs_refresh = true;
};

} // namespace zod::sodium
