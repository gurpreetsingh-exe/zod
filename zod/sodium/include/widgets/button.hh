#pragma once

#include "widget.hh"
#include "widgets/rect.hh"

namespace zod {

class SButton : public SWidget {
public:
  SButton(String n) : SWidget(std::move(n)) {}

public:
  auto on_event(Event&) -> void override;
  auto draw_imp(Geometry&) -> void override;
  auto compute_desired_size() -> void override;
};

} // namespace zod
