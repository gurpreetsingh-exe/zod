#include "application.hh"

namespace zod {

static SApplication* g_current = nullptr;

auto SApplication::create(SApplication* app) -> void { g_current = app; }

auto SApplication::get() -> SApplication& {
  ZASSERT(g_current != nullptr);
  return *g_current;
}

auto SApplication::active_window() -> SWindow& {
  ZASSERT(m_window.get());
  return *m_window;
}

auto SApplication::init_window(std::string name) -> void {
  ZASSERT(m_window == nullptr);
  m_window = SWindow::create(std::move(name));
  m_imgui_layer = unique<ImGuiLayer>(m_window->get_native_handle());
}

} // namespace zod
