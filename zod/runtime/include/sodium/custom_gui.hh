#pragma once

#include "application/event.hh"
#include "core/shapes.hh"
#include "sodium/gui.hh"

namespace zod::sodium::custom {

enum class IconId {
  None,
  Select,
  Move,
  Rotate,
  Scale,
  Minimize,
  Maximize,
  Close,
};

enum class DockSlot {
  Left,
  Right,
  Top,
  Bottom,
  Center,
  Floating,
};

enum class ResizeEdge {
  None,
  Left,
  Right,
  Top,
  Bottom,
};

struct Button {
  String label;
  IconId icon = IconId::None;
  Rect bounds = {};
  bool hovered = false;
  bool pressed = false;
  std::function<void()> on_click = {};

  explicit Button(String);
  Button(String, IconId);

  auto on_event(Event&) -> bool;
  auto paint(PaintCx&) const -> void;
};

struct MenuItem {
  String label;
  std::function<void()> action = {};
};

struct Menu {
  String label;
  Rect bounds = {};
  bool open = false;
  Vector<MenuItem> items = {};

  explicit Menu(String);

  auto add_item(String, std::function<void()> = {}) -> void;
  auto on_event(Event&) -> bool;
  auto paint(PaintCx&) const -> void;
};

struct DockPanel {
  using PaintFn = std::function<void(PaintCx&, const Rect&)>;

  String title;
  DockSlot slot = DockSlot::Center;
  Rect bounds = {};
  Rect content_bounds = {};
  bool visible = true;
  bool hovered = false;
  bool dragging = false;
  bool resizing = false;
  vec2 drag_offset = {};
  Rect resize_start = {};
  vec2 resize_mouse_start = {};
  ResizeEdge resize_edge = ResizeEdge::None;
  PaintFn paint_content = {};

  DockPanel(String, DockSlot = DockSlot::Center, PaintFn = {});

  auto title_bounds() const -> Rect;
  auto on_event(Event&) -> bool;
  auto paint(PaintCx&) const -> void;
};

struct DockSpace {
  enum class SplitKind {
    Leaf,
    Horizontal,
    Vertical,
  };

  struct DockNode {
    SplitKind kind = SplitKind::Leaf;
    DockPanel* panel = nullptr;
    UniquePtr<DockNode> first = nullptr;
    UniquePtr<DockNode> second = nullptr;
    f32 split = 0.5f;
    Rect bounds = {};
  };

  struct DockResizeTarget {
    DockNode* parent = nullptr;
    bool first_child = false;
    ResizeEdge edge = ResizeEdge::None;
  };

  Rect bounds = {};
  Vector<SharedPtr<DockPanel>> panels = {};
  Vector<Menu> menus = {};
  Vector<Button> toolbar = {};

  auto add_panel(SharedPtr<DockPanel>) -> SharedPtr<DockPanel>;
  auto add_menu(Menu) -> Menu&;
  auto add_button(Button) -> Button&;
  auto layout(Rect) -> void;
  auto on_event(Event&) -> bool;
  auto paint(PaintCx&) const -> void;

private:
  DockPanel* m_dragging = nullptr;
  DockPanel* m_resizing = nullptr;
  Rect m_drop_preview = {};
  DockSlot m_drop_slot = DockSlot::Floating;
  ResizeEdge m_resize_edge = ResizeEdge::None;
  vec2 m_resize_mouse_start = {};
  f32 m_resize_value_start = 0.0f;
  UniquePtr<DockNode> m_root = nullptr;
  DockNode* m_drop_node = nullptr;
  DockResizeTarget m_dock_resize = {};

  auto dock_slot_at(vec2) -> DockSlot;
  auto dock_rect(DockSlot) const -> Rect;
  auto find_node(DockNode*, DockPanel*) const -> DockNode*;
  auto find_node_at(DockNode*, vec2) const -> DockNode*;
  auto find_resize_target(DockNode*, DockPanel*, ResizeEdge) const
      -> DockResizeTarget;
  auto layout_node(DockNode*, Rect) -> void;
  auto remove_from_tree(DockPanel*) -> void;
  auto dock_panel(DockPanel*, DockNode*, DockSlot) -> void;
};

auto label(const String&, vec2, f32 scale = 0.75f) -> void;
auto icon(IconId, Rect, vec4 color = { 0.86f, 0.90f, 0.95f, 1.0f }) -> void;
auto flush_icons() -> void;

} // namespace zod::sodium::custom
