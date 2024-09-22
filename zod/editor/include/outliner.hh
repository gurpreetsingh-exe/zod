#pragma once

#include "backend.hh"
#include "widgets/panel.hh"

namespace zod {

class Outliner : public Panel {
public:
  Outliner() = default;
  auto on_event(Event& event) -> void override;
  auto draw(Geometry&) -> void override;
};

} // namespace zod
