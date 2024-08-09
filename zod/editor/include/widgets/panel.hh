#pragma once

#include "widget.hh"

namespace zod {

class Panel : public Widget {
public:
  Panel() = default;
  auto on_event(Event&) -> void override {}
  auto draw() -> void {}
};

} // namespace zod
