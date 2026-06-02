#pragma once

#include "sodium/widgets/compound_widget.hh"

namespace zod {

namespace sodium {
template <class ItemT>
class ListView;
} // namespace sodium

class Debug : public sodium::CompoundWidget {
public:
  Debug();

public:
  auto request_refresh() -> void;

private:
  SharedPtr<sodium::ListView<std::pair<String, f32>>> m_debug_timings;
  SharedPtr<sodium::ListView<String>> m_hover_paths;
};

} // namespace zod
