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

protected:
  Unique<Layout> m_layout;
  Unique<SWindow> m_window;
  Unique<ImGuiLayer> m_imgui_layer;
};

} // namespace zod
