#pragma once

#include "widgets/panel.hh"

namespace zod {

class Properties : public Panel {
public:
  Properties();
  auto update() -> void;

private:
  auto draw_imp(Geometry&) -> void override { update(); }
};

} // namespace zod
