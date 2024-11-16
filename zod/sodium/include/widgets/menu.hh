#pragma once

#include "widget.hh"
#include "widgets/button.hh"

namespace zod {

class SMenu : public SWidget {
public:
  SMenu(std::string n) : SWidget(std::move(n)) {}

public:
  auto on_event(Event&) -> void override;
  auto draw_imp(Geometry&) -> void override;

public:
  auto compute_desired_size() -> void override;
  auto add_button(std::string name) -> void {
    auto button = shared<SButton>(name);
    m_children.push_back(button);
  }

private:
  std::vector<Shared<SWidget>> m_children = {};
};

} // namespace zod