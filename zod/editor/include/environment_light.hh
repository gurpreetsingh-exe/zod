#pragma once

#include "widgets/panel.hh"

namespace zod {

class EnvironmentLight : public SPanel {
public:
  EnvironmentLight();
  auto on_event(Event&) -> void override;

private:
  auto draw_imp(Geometry&) -> void override;
};

} // namespace zod
