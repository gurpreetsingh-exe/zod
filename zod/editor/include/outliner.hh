#pragma once

#include "widgets/panel.hh"

namespace zod {

class Outliner : public Panel {
public:
  Outliner() : Panel("Outliner", unique<OrthographicCamera>(64.0f, 64.0f)) {}
  auto on_event(Event& event) -> void override;

private:
  auto draw_imp(Geometry&) -> void override;
};

} // namespace zod
