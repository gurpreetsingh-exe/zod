#pragma once

#include "widgets/panel.hh"

namespace zod {

class EnvironmentLight : public SPanel {
public:
  EnvironmentLight()
      : SPanel("Environment Light", unique<OrthographicCamera>(64.0f, 64.0f)) {}
  auto on_event(Event& event) -> void override;

private:
  auto draw_imp(Geometry&) -> void override;
};

} // namespace zod
