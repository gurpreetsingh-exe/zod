#pragma once

#include "imgui_layer.hh"
#include "widgets/layout.hh"
#include "window.hh"

namespace zod {

class SApplication {
public:
  SApplication() = default;
  virtual ~SApplication() = default;

public:
  static auto create(SApplication*) -> void;
  static auto get() -> SApplication&;
  auto init_window(std::string) -> void;
  auto active_window() -> SWindow&;
  auto on_event(Event&) -> void;
  auto working_directory() const -> const fs::path& {
    return m_working_directory;
  }

protected:
  Unique<Layout> m_layout;
  Unique<SWindow> m_window;
  Unique<ImGuiLayer> m_imgui_layer;
  fs::path m_working_directory;
};

} // namespace zod
