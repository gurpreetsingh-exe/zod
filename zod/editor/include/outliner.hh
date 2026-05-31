#pragma once

#include "engine/entity.hh"
#include "sodium/widgets/compound_widget.hh"
#include "sodium/widgets/multi_column_list_view.hh"

namespace zod {

class Outliner : public sodium::CompoundWidget {
public:
  Outliner();

  auto request_refresh() -> void;

private:
  static auto type_name(Entity) -> const char*;
  static auto text_cell(String) -> SharedPtr<sodium::Widget>;

private:
  SharedPtr<sodium::MultiColumnListView<Entity>> m_list = nullptr;
};

} // namespace zod
