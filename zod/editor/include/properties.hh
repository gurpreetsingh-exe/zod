#pragma once

#include "widgets/panel.hh"

namespace zod {

class Properties : public SPanel {
public:
  Properties();
  auto update() -> void;

private:
  auto draw_imp(Geometry&) -> void override { update(); }
};

} // namespace zod
