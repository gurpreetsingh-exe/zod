#pragma once

#include "engine/entity.hh"
#include "widgets/panel.hh"

namespace zod {

class Outliner : public SPanel {
public:
  Outliner()
      : SPanel("Outliner", unique<OrthographicCamera>(64.0f, 64.0f), false) {}
  auto on_event(Event& event) -> void override;

private:
  auto draw_imp(Geometry&) -> void override;
  auto draw_entity(Entity) -> void;
};

} // namespace zod
