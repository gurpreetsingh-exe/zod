#pragma once

#pragma once

#include "widgets/panel.hh"

namespace zod {

class ContentBrowser : public SPanel {
public:
  ContentBrowser();
  auto update() -> void;

private:
  auto draw_imp(Geometry&) -> void override { update(); }

private:
  fs::path m_base_directory;
  fs::path m_current_directory;
};

} // namespace zod
