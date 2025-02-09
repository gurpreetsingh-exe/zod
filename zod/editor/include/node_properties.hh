#pragma once

#include "widgets/panel.hh"

namespace zod {

class NodeProperties : public SPanel {
public:
  NodeProperties();
  auto update() -> void;

private:
  auto draw_imp(Geometry&) -> void override { update(); }
};

} // namespace zod
