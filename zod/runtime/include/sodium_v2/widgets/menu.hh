#pragma once

#include "sodium_v2/widgets/widget.hh"

namespace zod::sodium {

class Container;

class Menu : public Widget {
public:
  using Action = std::function<void()>;

  Menu();
  ~Menu() override;

  auto add_entry(String label, Action action = {}) -> Menu&;
  auto add_separator() -> Menu&;
  auto clear_entries() -> void;
  auto set_close_after_selection(bool close) -> void {
    m_close_after_selection = close;
  }
  auto close_after_selection() const -> bool { return m_close_after_selection; }
  auto open() -> void;
  auto close() -> void;
  auto toggle() -> void;
  auto is_open() const -> bool { return m_open; }
  static auto dismiss_all() -> void;

  auto on_mouse_down(const Event&) -> EventResponse override;
  auto on_mouse_enter(const Event&) -> void override;
  auto on_key_down(const Event&) -> EventResponse override;
  auto compute_desired_size(vec2) -> vec2 override;
  auto paint(PaintCx&) const -> void override;
  auto arrange(const Rect&) -> void override;
  auto get_children() const -> WidgetChildren override;

private:
  friend class MenuStack;

  auto set_open_from_stack(bool open) -> void;
  auto arrange_popup() -> void;

private:
  bool m_open = false;
  bool m_close_after_selection = true;
  vec2 m_popup_size = {};
  SharedPtr<Widget> m_dismiss_layer = nullptr;
  SharedPtr<Widget> m_popup_root = nullptr;
  SharedPtr<Container> m_popup_content = nullptr;
};

} // namespace zod::sodium
