#include "sodium/custom_gui.hh"

#include <cmath>

#include "application/application.hh"
#include "application/input.hh"
#include "application/window.hh"
#include "gpu/backend.hh"

namespace zod::sodium {
extern mat4 projection;
}

namespace zod::sodium::custom {

namespace {

constexpr auto TitleHeight = 28.0f;
constexpr auto MenuHeight = 28.0f;
constexpr auto ToolbarHeight = 34.0f;
constexpr auto Padding = 6.0f;
constexpr auto DockGap = 5.0f;
constexpr auto DockEdgeHitSize = 28.0f;
constexpr auto ResizeHandle = 5.0f;
constexpr auto MinPanelWidth = 180.0f;
constexpr auto MinPanelHeight = 120.0f;

constexpr auto AppBg = vec4 { 0.015f, 0.017f, 0.020f, 1.0f };
constexpr auto TitleBg = vec4 { 0.028f, 0.031f, 0.036f, 1.0f };
constexpr auto MenuBg = vec4 { 0.040f, 0.043f, 0.050f, 1.0f };
constexpr auto ToolbarBg = vec4 { 0.030f, 0.034f, 0.040f, 1.0f };
constexpr auto PanelBg = vec4 { 0.032f, 0.036f, 0.043f, 1.0f };
constexpr auto PanelBody = vec4 { 0.048f, 0.053f, 0.062f, 1.0f };
constexpr auto PanelHeader = vec4 { 0.030f, 0.034f, 0.040f, 1.0f };
constexpr auto PanelHeaderHot = vec4 { 0.055f, 0.065f, 0.078f, 1.0f };
constexpr auto ButtonBg = vec4 { 0.070f, 0.076f, 0.086f, 1.0f };
constexpr auto ButtonHot = vec4 { 0.095f, 0.105f, 0.120f, 1.0f };
constexpr auto Accent = vec4 { 0.000f, 0.475f, 0.780f, 1.0f };
constexpr auto Border = vec4 { 0.006f, 0.008f, 0.011f, 1.0f };
constexpr auto BorderLight = vec4 { 0.125f, 0.135f, 0.150f, 1.0f };
constexpr auto PopupBg = vec4 { 0.022f, 0.025f, 0.030f, 1.0f };
constexpr auto PopupItemHot = vec4 { 0.070f, 0.095f, 0.120f, 1.0f };
constexpr auto Preview = vec4 { 0.000f, 0.475f, 0.780f, 0.38f };
constexpr auto PreviewLine = vec4 { 0.000f, 0.620f, 1.000f, 0.85f };

struct TextCommand {
  String text;
  vec2 position = {};
  f32 scale = 1.0f;
  i32 layer = 0;
};

auto g_text_commands = Vector<TextCommand>();
auto g_text_layer = 0;

constexpr auto IconAtlasColumns = 8.0f;
constexpr auto IconAtlasRows = 1.0f;

auto icon_index(IconId id) -> i32 {
  switch (id) {
    case IconId::Select:
      return 0;
    case IconId::Move:
      return 1;
    case IconId::Rotate:
      return 2;
    case IconId::Scale:
      return 3;
    case IconId::Minimize:
      return 4;
    case IconId::Maximize:
      return 5;
    case IconId::Close:
      return 6;
    case IconId::None:
      return -1;
  }
  return -1;
}

auto draw_rect(PaintCx& cx, Rect rect, vec4 color) -> void {
  ::zod::sodium::rect(rect, color).paint(cx);
}

auto textured_rect(PaintCx& cx, Rect rect, Rect uv, vec4 color) -> void {
  auto start = cx.draw_list.draw_data.size();
  cx.draw_list.draw_data.push_back({ rect.position + vec2 { 0.0f, rect.size.y },
                                     uv.position + vec2 { 0.0f, uv.size.y } });
  cx.draw_list.draw_data.push_back(
      { rect.position + rect.size, uv.position + uv.size });
  cx.draw_list.draw_data.push_back({ rect.position + vec2 { rect.size.x, 0.0f },
                                     uv.position + vec2 { uv.size.x, 0.0f } });
  cx.draw_list.draw_data.push_back({ rect.position, uv.position });
  cx.draw_list.indices.push_back(start);
  cx.draw_list.indices.push_back(start + 1);
  cx.draw_list.indices.push_back(start + 2);
  cx.draw_list.indices.push_back(start + 2);
  cx.draw_list.indices.push_back(start + 3);
  cx.draw_list.indices.push_back(start);
  cx.draw_list.colors.push_back(color);
  cx.draw_list.colors.push_back(color);
}

auto centered_square(Rect rect, f32 side) -> Rect {
  side = std::min(side, std::min(rect.size.x, rect.size.y));
  return { rect.position + (rect.size - vec2 { side, side }) * 0.5f,
           { side, side } };
}

auto resize_edge_at(Rect rect, vec2 mouse) -> ResizeEdge {
  if (not rect.margin(ResizeHandle).intersect(mouse)) {
    return ResizeEdge::None;
  }

  auto left = std::abs(mouse.x - rect.position.x);
  auto right = std::abs(mouse.x - (rect.position.x + rect.size.x));
  auto top = std::abs(mouse.y - rect.position.y);
  auto bottom = std::abs(mouse.y - (rect.position.y + rect.size.y));
  auto best = std::min(std::min(left, right), std::min(top, bottom));

  if (best > ResizeHandle) {
    return ResizeEdge::None;
  }
  if (best == left) {
    return ResizeEdge::Left;
  }
  if (best == right) {
    return ResizeEdge::Right;
  }
  if (best == top) {
    return ResizeEdge::Top;
  }
  return ResizeEdge::Bottom;
}

auto dock_resize_edge(DockSlot slot, Rect rect, vec2 mouse) -> ResizeEdge {
  auto edge = resize_edge_at(rect, mouse);
  switch (slot) {
    case DockSlot::Left:
      return edge == ResizeEdge::Right ? edge : ResizeEdge::None;
    case DockSlot::Right:
      return edge == ResizeEdge::Left ? edge : ResizeEdge::None;
    case DockSlot::Top:
      return edge == ResizeEdge::Bottom ? edge : ResizeEdge::None;
    case DockSlot::Bottom:
      return edge == ResizeEdge::Top ? edge : ResizeEdge::None;
    case DockSlot::Center:
    case DockSlot::Floating:
      return ResizeEdge::None;
  }
  return ResizeEdge::None;
}

auto is_horizontal_edge(ResizeEdge edge) -> bool {
  return edge == ResizeEdge::Left or edge == ResizeEdge::Right;
}

auto cursor_for_edge(ResizeEdge edge) -> cursor_shape_t {
  switch (edge) {
    case ResizeEdge::Left:
    case ResizeEdge::Right:
      return cursor_shape_t::ResizeHorizontal;
    case ResizeEdge::Top:
    case ResizeEdge::Bottom:
      return cursor_shape_t::ResizeVertical;
    case ResizeEdge::None:
      return cursor_shape_t::Arrow;
  }
  return cursor_shape_t::Arrow;
}

auto content_rect(Rect bounds) -> Rect {
  auto content = bounds;
  content.position.y += 32.0f + MenuHeight + ToolbarHeight;
  content.size.y -= 32.0f + MenuHeight + ToolbarHeight;
  return content;
}

auto with_dock_gap(Rect rect) -> Rect { return rect.padding(DockGap * 0.5f); }

auto dock_edge_at(Rect rect, vec2 mouse) -> DockSlot {
  if (not rect.intersect(mouse)) {
    return DockSlot::Floating;
  }

  auto local = mouse - rect.position;
  auto left = local.x;
  auto right = rect.size.x - local.x;
  auto top = local.y;
  auto bottom = rect.size.y - local.y;
  auto closest = std::min(std::min(left, right), std::min(top, bottom));

  if (closest > DockEdgeHitSize) {
    return DockSlot::Floating;
  }
  if (closest == left) {
    return DockSlot::Left;
  }
  if (closest == right) {
    return DockSlot::Right;
  }
  if (closest == top) {
    return DockSlot::Top;
  }
  return DockSlot::Bottom;
}

auto dock_hint_rect(Rect rect, DockSlot slot) -> Rect {
  constexpr auto Thickness = 8.0f;
  switch (slot) {
    case DockSlot::Left:
      return { rect.position, { Thickness, rect.size.y } };
    case DockSlot::Right:
      return { { rect.position.x + rect.size.x - Thickness, rect.position.y },
               { Thickness, rect.size.y } };
    case DockSlot::Top:
      return { rect.position, { rect.size.x, Thickness } };
    case DockSlot::Bottom:
      return { { rect.position.x, rect.position.y + rect.size.y - Thickness },
               { rect.size.x, Thickness } };
    case DockSlot::Center:
    case DockSlot::Floating:
      break;
  }
  return {};
}

auto draw_outline(PaintCx& cx, Rect rect, vec4 color) -> void {
  constexpr auto Thickness = 1.0f;
  draw_rect(cx, { rect.position, { rect.size.x, Thickness } }, color);
  draw_rect(cx,
            { { rect.position.x, rect.position.y + rect.size.y - Thickness },
              { rect.size.x, Thickness } },
            color);
  draw_rect(cx, { rect.position, { Thickness, rect.size.y } }, color);
  draw_rect(cx,
            { { rect.position.x + rect.size.x - Thickness, rect.position.y },
              { Thickness, rect.size.y } },
            color);
}

struct IconAtlas {
  SharedPtr<GPUTexture> texture = nullptr;
  PaintCx cx = {};

  IconAtlas() {
    auto path = fs::path("icons/blender_icons.png");
    if (not fs::exists(path)) {
      path = fs::path("build-release/icons/blender_icons.png");
    }
    texture = GPUBackend::get().create_texture({
        .name = "sodium.custom.blender_icons",
        .wrap = GPUTextureWrap::Clamp,
        .path = path,
    });
  }

  auto uv(IconId id) const -> Rect {
    auto idx = icon_index(id);
    if (idx < 0) {
      return {};
    }
    auto w = 1.0f / IconAtlasColumns;
    auto h = 1.0f / IconAtlasRows;
    return { { f32(idx) * w, 0.0f }, { w, h } };
  }
};

auto icon_atlas() -> IconAtlas& {
  static auto atlas = IconAtlas();
  return atlas;
}

auto mouse_from(Event& event) -> vec2 {
  if (event.kind == Event::MouseMove or event.kind == Event::MouseDown or
      event.kind == Event::MouseUp) {
    return event.mouse;
  }
  return Input::get_mouse_pos();
}

auto text_width(const String& text, f32 scale) -> f32 {
  return f32(text.size()) * scale * 0.46f;
}

auto draw_label(const String& text, vec2 position, f32 scale) -> void {
  g_text_commands.push_back({ text, position, scale, g_text_layer });
}

auto flush_labels(i32 layer) -> void {
  for (const auto& command : g_text_commands) {
    if (command.layer != layer) {
      continue;
    }
    Font::get().render_text(command.text.c_str(), command.position.x,
                            command.position.y, command.scale, command.scale);
  }
  Font::get().submit();
}

auto paint_menu_button(const Menu& menu, PaintCx& cx) -> void {
  draw_rect(cx, menu.bounds, menu.open ? PanelHeaderHot : MenuBg);
  if (menu.open) {
    draw_rect(cx,
              { { menu.bounds.position.x,
                  menu.bounds.position.y + menu.bounds.size.y - 2.0f },
                { menu.bounds.size.x, 2.0f } },
              Accent);
  }
  draw_label(menu.label, menu.bounds.position + vec2 { 10.0f, 19.0f }, 0.72f);
}

auto paint_menu_popup(const Menu& menu, PaintCx& cx) -> void {
  if (not menu.open) {
    return;
  }

  auto mouse = Input::get_mouse_pos();
  auto item_area =
      Rect { menu.bounds.position + vec2 { 0.0f, menu.bounds.size.y },
             { 210.0f, f32(menu.items.size()) * 28.0f + 8.0f } };
  draw_rect(cx, item_area.margin(1.0f), Border);
  draw_rect(cx, item_area, PopupBg);
  draw_rect(cx, { item_area.position, { item_area.size.x, 2.0f } }, Accent);

  for (usize i = 0; i < menu.items.size(); ++i) {
    auto row = Rect {
      item_area.position + vec2 { 4.0f, 4.0f + f32(i) * 28.0f },
      { item_area.size.x - 8.0f, 26.0f },
    };
    if (row.intersect(mouse)) {
      draw_rect(cx, row, PopupItemHot);
    }
    auto pos = row.position + vec2 { 12.0f, 19.0f };
    draw_label(menu.items[i].label, pos, 0.70f);
  }
}

auto window_button_rect(Rect bounds, usize index) -> Rect {
  constexpr auto Width = 45.0f;
  return { { bounds.position.x + bounds.size.x - Width * f32(index + 1),
             bounds.position.y },
           { Width, 32.0f } };
}

} // namespace

auto label(const String& text, vec2 position, f32 scale) -> void {
  draw_label(text, position, scale);
}

auto icon(IconId id, Rect bounds, vec4 color) -> void {
  if (id == IconId::None) {
    return;
  }
  auto& atlas = icon_atlas();
  textured_rect(atlas.cx, bounds, atlas.uv(id), color);
}

auto flush_icons() -> void {
  auto& atlas = icon_atlas();
  if (not atlas.cx.draw_list.indices.empty()) {
    ::zod::sodium::submit(atlas.cx, atlas.texture);
  }
}

Button::Button(String label_) : label(std::move(label_)) {}
Button::Button(String label_, IconId icon_)
    : label(std::move(label_)), icon(icon_) {}

auto Button::on_event(Event& event) -> bool {
  auto mouse = mouse_from(event);
  hovered = bounds.intersect(mouse);
  if (event.kind == Event::MouseDown and event.button == MouseButton::Left and
      hovered) {
    pressed = true;
    event.hanging = false;
    return true;
  }
  if (event.kind == Event::MouseUp and event.button == MouseButton::Left) {
    auto was_pressed = pressed;
    pressed = false;
    if (was_pressed and hovered) {
      if (on_click) {
        on_click();
      }
      event.hanging = false;
      return true;
    }
  }
  return hovered;
}

auto Button::paint(PaintCx& cx) const -> void {
  auto color = pressed ? Accent : hovered ? ButtonHot : ButtonBg;
  draw_rect(cx, bounds, color);
  if (icon != IconId::None) {
    auto side = std::min(bounds.size.x, bounds.size.y) - 8.0f;
    auto offset = (bounds.size - vec2 { side, side }) * 0.5f;
    ::zod::sodium::custom::icon(icon,
                                { bounds.position + offset, { side, side } });
  } else {
    draw_label(label, bounds.position + vec2 { 10.0f, 20.0f }, 0.72f);
  }
}

Menu::Menu(String label_) : label(std::move(label_)) {}

auto Menu::add_item(String item, std::function<void()> action) -> void {
  items.push_back({ std::move(item), std::move(action) });
}

auto Menu::on_event(Event& event) -> bool {
  auto mouse = mouse_from(event);
  auto hovered = bounds.intersect(mouse);
  auto item_area = Rect { bounds.position + vec2 { 0.0f, bounds.size.y },
                          { 210.0f, f32(items.size()) * 28.0f + 8.0f } };

  if (event.kind == Event::MouseDown and event.button == MouseButton::Left) {
    if (hovered) {
      open = not open;
      event.hanging = false;
      return true;
    }

    if (open and item_area.intersect(mouse)) {
      auto idx = usize((mouse.y - item_area.position.y - 4.0f) / 28.0f);
      if (idx < items.size() and items[idx].action) {
        items[idx].action();
      }
      open = false;
      event.hanging = false;
      return true;
    }

    open = false;
  }

  return hovered or (open and item_area.intersect(mouse));
}

auto Menu::paint(PaintCx& cx) const -> void {
  paint_menu_button(*this, cx);
  paint_menu_popup(*this, cx);
}

DockPanel::DockPanel(String title_, DockSlot slot_, PaintFn paint_content_)
    : title(std::move(title_)), slot(slot_),
      paint_content(std::move(paint_content_)) {}

auto DockPanel::title_bounds() const -> Rect {
  return { bounds.position, { bounds.size.x, TitleHeight } };
}

auto DockPanel::on_event(Event& event) -> bool {
  if (not visible) {
    return false;
  }

  auto mouse = mouse_from(event);
  hovered = bounds.intersect(mouse);

  if (resizing and event.kind == Event::MouseMove) {
    auto delta = mouse - resize_mouse_start;
    bounds = resize_start;
    switch (resize_edge) {
      case ResizeEdge::Left:
        bounds.position.x = resize_start.position.x + delta.x;
        bounds.size.x = resize_start.size.x - delta.x;
        break;
      case ResizeEdge::Right:
        bounds.size.x = resize_start.size.x + delta.x;
        break;
      case ResizeEdge::Top:
        bounds.position.y = resize_start.position.y + delta.y;
        bounds.size.y = resize_start.size.y - delta.y;
        break;
      case ResizeEdge::Bottom:
        bounds.size.y = resize_start.size.y + delta.y;
        break;
      case ResizeEdge::None:
        break;
    }
    if (bounds.size.x < MinPanelWidth) {
      bounds.size.x = MinPanelWidth;
      if (resize_edge == ResizeEdge::Left) {
        bounds.position.x =
            resize_start.position.x + resize_start.size.x - MinPanelWidth;
      }
    }
    if (bounds.size.y < MinPanelHeight) {
      bounds.size.y = MinPanelHeight;
      if (resize_edge == ResizeEdge::Top) {
        bounds.position.y =
            resize_start.position.y + resize_start.size.y - MinPanelHeight;
      }
    }
    content_bounds =
        Rect { bounds.position + vec2 { Padding, TitleHeight + Padding },
               bounds.size -
                   vec2 { Padding * 2.0f, TitleHeight + Padding * 2.0f } };
    event.hanging = false;
    return true;
  }

  if (resizing and event.kind == Event::MouseUp and
      event.button == MouseButton::Left) {
    resizing = false;
    resize_edge = ResizeEdge::None;
    event.hanging = false;
    return true;
  }

  if (slot == DockSlot::Floating and event.kind == Event::MouseDown and
      event.button == MouseButton::Left) {
    auto edge = resize_edge_at(bounds, mouse);
    if (edge != ResizeEdge::None) {
      resizing = true;
      resize_edge = edge;
      resize_start = bounds;
      resize_mouse_start = mouse;
      event.hanging = false;
      return true;
    }
  }

  if (event.kind == Event::MouseDown and event.button == MouseButton::Left and
      title_bounds().intersect(mouse)) {
    dragging = true;
    drag_offset = mouse - bounds.position;
    slot = DockSlot::Floating;
    event.hanging = false;
    return true;
  }

  if (dragging and event.kind == Event::MouseMove) {
    bounds.position = mouse - drag_offset;
    content_bounds = bounds.padding({ Padding, TitleHeight + Padding });
    event.hanging = false;
    return true;
  }

  if (dragging and event.kind == Event::MouseUp and
      event.button == MouseButton::Left) {
    dragging = false;
    event.hanging = false;
    return true;
  }

  return hovered;
}

auto DockPanel::paint(PaintCx& cx) const -> void {
  if (not visible) {
    return;
  }

  draw_rect(cx, bounds, PanelBody);
  draw_rect(cx, title_bounds(),
            dragging  ? Accent
            : hovered ? PanelHeaderHot
                      : PanelHeader);
  draw_rect(cx,
            { { bounds.position.x, bounds.position.y + TitleHeight - 1.0f },
              { bounds.size.x, 1.0f } },
            Border);
  draw_label(title, bounds.position + vec2 { 10.0f, 20.0f }, 0.74f);

  if (paint_content) {
    paint_content(cx, content_bounds);
  }
}

auto DockSpace::add_panel(SharedPtr<DockPanel> panel) -> SharedPtr<DockPanel> {
  auto* raw_panel = panel.get();
  panels.push_back(panel);
  if (panel->slot != DockSlot::Floating) {
    if (not m_root) {
      m_root = unique<DockNode>();
      m_root->panel = raw_panel;
      panel->slot = DockSlot::Center;
    } else {
      auto slot =
          panel->slot == DockSlot::Center ? DockSlot::Right : panel->slot;
      dock_panel(raw_panel, m_root.get(), slot);
    }
  }
  return panel;
}

auto DockSpace::add_menu(Menu menu) -> Menu& {
  menus.push_back(std::move(menu));
  return menus.back();
}

auto DockSpace::add_button(Button button) -> Button& {
  toolbar.push_back(std::move(button));
  return toolbar.back();
}

auto DockSpace::layout(Rect new_bounds) -> void {
  bounds = new_bounds;

  auto menu_x = bounds.position.x + 8.0f;
  for (auto& menu : menus) {
    auto width = std::max(56.0f, text_width(menu.label, 0.72f) + 22.0f);
    menu.bounds = { { menu_x, bounds.position.y + 32.0f },
                    { width, MenuHeight } };
    menu_x += width;
  }

  auto button_x = bounds.position.x + Padding;
  for (auto& button : toolbar) {
    auto width = button.icon != IconId::None
                     ? 31.0f
                     : std::max(82.0f, text_width(button.label, 0.72f) + 28.0f);
    button.bounds = { { button_x,
                        bounds.position.y + 32.0f + MenuHeight + 4.0f },
                      { width, 25.0f } };
    button_x += width + 6.0f;
  }

  auto content = content_rect(bounds);
  if (m_root) {
    layout_node(m_root.get(), content);
  }

  for (auto& panel : panels) {
    if (panel->slot == DockSlot::Floating or panel->dragging) {
      if (panel->bounds.size.x <= 0.0f or panel->bounds.size.y <= 0.0f) {
        panel->bounds = { content.position + vec2 { 80.0f, 80.0f },
                          { 360.0f, 260.0f } };
      }
    }
    panel->content_bounds =
        Rect { panel->bounds.position + vec2 { Padding, TitleHeight + Padding },
               panel->bounds.size -
                   vec2 { Padding * 2.0f, TitleHeight + Padding * 2.0f } };
  }
}

auto DockSpace::on_event(Event& event) -> bool {
  auto content = content_rect(bounds);

  if (event.kind == Event::MouseMove and not m_dragging and not m_resizing) {
    auto mouse = mouse_from(event);
    auto edge = ResizeEdge::None;
    for (auto it = panels.rbegin(); it != panels.rend(); ++it) {
      auto& panel = *it;
      edge = panel->resizing ? panel->resize_edge
                             : resize_edge_at(panel->bounds, mouse);
      if (panel->slot != DockSlot::Floating) {
        edge = find_resize_target(m_root.get(), panel.get(), edge).edge;
      }
      if (edge != ResizeEdge::None) {
        break;
      }
    }
    Application::get().active_window().set_cursor(cursor_for_edge(edge));
  }

  if (event.kind == Event::MouseDown and event.button == MouseButton::Left) {
    auto mouse = mouse_from(event);
    if (window_button_rect(bounds, 0).intersect(mouse)) {
      auto close = Event { .kind = Event::WindowClose };
      Application::get().on_event(close);
      event.hanging = false;
      return true;
    }
  }

  if (m_dragging and event.kind == Event::MouseMove) {
    m_dragging->bounds.position = mouse_from(event) - m_dragging->drag_offset;
    m_dragging->content_bounds =
        Rect { m_dragging->bounds.position +
                   vec2 { Padding, TitleHeight + Padding },
               m_dragging->bounds.size -
                   vec2 { Padding * 2.0f, TitleHeight + Padding * 2.0f } };
    auto slot = dock_slot_at(mouse_from(event));
    m_drop_slot = slot;
    m_drop_preview = slot == DockSlot::Floating ? Rect() : dock_rect(slot);
    event.hanging = false;
    return true;
  }

  if (m_resizing and event.kind == Event::MouseMove) {
    auto delta = mouse_from(event) - m_resize_mouse_start;
    if (m_dock_resize.parent) {
      auto* node = m_dock_resize.parent;
      if (node->kind == SplitKind::Horizontal) {
        auto change = delta.x / std::max(1.0f, node->bounds.size.x);
        node->split =
            glm::clamp(m_resize_value_start +
                           (m_dock_resize.first_child ? change : -change),
                       0.15f, 0.85f);
      } else if (node->kind == SplitKind::Vertical) {
        auto change = delta.y / std::max(1.0f, node->bounds.size.y);
        node->split =
            glm::clamp(m_resize_value_start +
                           (m_dock_resize.first_child ? -change : change),
                       0.15f, 0.85f);
      }
      layout(bounds);
      Application::get().active_window().set_cursor(
          cursor_for_edge(m_resize_edge));
      event.hanging = false;
      return true;
    }
  }

  if (m_resizing and event.kind == Event::MouseUp and
      event.button == MouseButton::Left) {
    m_resizing = nullptr;
    m_resize_edge = ResizeEdge::None;
    m_dock_resize = {};
    Application::get().active_window().set_cursor(cursor_shape_t::Arrow);
    event.hanging = false;
    return true;
  }

  if (m_dragging and event.kind == Event::MouseUp and
      event.button == MouseButton::Left) {
    auto slot = dock_slot_at(mouse_from(event));
    if (slot != DockSlot::Floating) {
      dock_panel(m_dragging, m_drop_node, slot);
    } else {
      m_dragging->slot = DockSlot::Floating;
    }
    m_dragging->dragging = false;
    m_dragging = nullptr;
    m_drop_node = nullptr;
    m_drop_slot = DockSlot::Floating;
    layout(bounds);
    Application::get().active_window().set_cursor(cursor_shape_t::Arrow);
    event.hanging = false;
    return true;
  }

  for (auto& menu : menus) {
    if (menu.on_event(event)) {
      return true;
    }
  }

  for (auto& button : toolbar) {
    if (button.on_event(event)) {
      return true;
    }
  }

  if (event.kind == Event::MouseDown and event.button == MouseButton::Left) {
    auto mouse = mouse_from(event);
    for (auto it = panels.rbegin(); it != panels.rend(); ++it) {
      auto& panel = *it;
      if (panel->slot == DockSlot::Floating) {
        continue;
      }
      auto edge = resize_edge_at(panel->bounds, mouse);
      auto target = find_resize_target(m_root.get(), panel.get(), edge);
      if (edge == ResizeEdge::None or not target.parent) {
        continue;
      }
      m_resizing = panel.get();
      m_resize_edge = edge;
      m_resize_mouse_start = mouse;
      m_resize_value_start = target.parent->split;
      m_dock_resize = target;
      Application::get().active_window().set_cursor(cursor_for_edge(edge));
      event.hanging = false;
      return true;
    }
  }

  for (auto it = panels.rbegin(); it != panels.rend(); ++it) {
    auto& panel = *it;
    if (panel->on_event(event)) {
      if (panel->dragging) {
        remove_from_tree(panel.get());
        m_dragging = panel.get();
        auto slot = dock_slot_at(mouse_from(event));
        m_drop_slot = slot;
        m_drop_preview = slot == DockSlot::Floating ? Rect() : dock_rect(slot);
      }
      if (event.kind == Event::MouseUp and not panel->resizing) {
        Application::get().active_window().set_cursor(cursor_shape_t::Arrow);
      }
      return true;
    }
  }

  return bounds.intersect(mouse_from(event));
}

auto DockSpace::paint(PaintCx& cx) const -> void {
  g_text_commands.clear();
  g_text_layer = 0;

  draw_rect(cx, bounds, AppBg);
  draw_rect(cx, { bounds.position, { bounds.size.x, 32.0f } }, TitleBg);
  draw_rect(cx, { bounds.position, { 4.0f, 32.0f } }, Accent);
  draw_rect(cx,
            { { bounds.position.x, bounds.position.y + 31.0f },
              { bounds.size.x, 1.0f } },
            Border);
  draw_label("Zod Editor", bounds.position + vec2 { 12.0f, 22.0f }, 0.78f);

  auto mouse = Input::get_mouse_pos();
  auto close = window_button_rect(bounds, 0);
  auto maximize = window_button_rect(bounds, 1);
  auto minimize = window_button_rect(bounds, 2);
  draw_rect(cx, minimize, minimize.intersect(mouse) ? ButtonHot : TitleBg);
  draw_rect(cx, maximize, maximize.intersect(mouse) ? ButtonHot : TitleBg);
  draw_rect(cx, close,
            close.intersect(mouse) ? vec4 { 0.78f, 0.08f, 0.08f, 1.0f }
                                   : TitleBg);
  icon(IconId::Minimize, centered_square(minimize, 14.0f));
  icon(IconId::Maximize, centered_square(maximize, 14.0f));
  icon(IconId::Close, centered_square(close, 14.0f));

  draw_rect(
      cx,
      { bounds.position + vec2 { 0.0f, 32.0f }, { bounds.size.x, MenuHeight } },
      MenuBg);
  draw_rect(cx,
            { bounds.position + vec2 { 0.0f, 32.0f + MenuHeight },
              { bounds.size.x, ToolbarHeight } },
            ToolbarBg);
  draw_rect(cx,
            { bounds.position +
                  vec2 { 0.0f, 32.0f + MenuHeight + ToolbarHeight - 1.0f },
              { bounds.size.x, 1.0f } },
            Border);

  for (const auto& menu : menus) { paint_menu_button(menu, cx); }

  for (const auto& button : toolbar) { button.paint(cx); }

  for (const auto& panel : panels) {
    if (panel->slot != DockSlot::Floating) {
      panel->paint(cx);
    }
  }

  for (const auto& panel : panels) {
    if (panel->slot == DockSlot::Floating) {
      panel->paint(cx);
    }
  }

  if (m_dragging and m_drop_preview.size.x > 0.0f and
      m_drop_preview.size.y > 0.0f) {
    draw_rect(cx, m_drop_preview, Preview);
    draw_rect(cx, dock_hint_rect(m_drop_preview, m_drop_slot), PreviewLine);
    draw_outline(cx, m_drop_preview, PreviewLine);
  }

  ::zod::sodium::submit(cx);
  flush_icons();
  flush_labels(0);

  auto popup_cx = PaintCx();
  g_text_layer = 1;
  for (const auto& menu : menus) { paint_menu_popup(menu, popup_cx); }
  ::zod::sodium::submit(popup_cx);
  flush_labels(1);
  g_text_commands.clear();
}

auto DockSpace::find_node(DockNode* node, DockPanel* panel) const -> DockNode* {
  if (not node) {
    return nullptr;
  }
  if (node->kind == SplitKind::Leaf) {
    return node->panel == panel ? node : nullptr;
  }
  if (auto* found = find_node(node->first.get(), panel)) {
    return found;
  }
  return find_node(node->second.get(), panel);
}

auto DockSpace::find_node_at(DockNode* node, vec2 mouse) const -> DockNode* {
  if (not node or not node->bounds.intersect(mouse)) {
    return nullptr;
  }
  if (node->kind == SplitKind::Leaf) {
    return node;
  }
  if (auto* found = find_node_at(node->first.get(), mouse)) {
    return found;
  }
  return find_node_at(node->second.get(), mouse);
}

auto DockSpace::find_resize_target(DockNode* node, DockPanel* panel,
                                   ResizeEdge desired_edge) const
    -> DockResizeTarget {
  if (desired_edge == ResizeEdge::None) {
    return {};
  }

  auto find_impl = [&](auto&& self,
                       DockNode* current) -> std::pair<bool, DockResizeTarget> {
    if (not current) {
      return { false, {} };
    }

    if (current->kind == SplitKind::Leaf and current->panel == panel) {
      return { true, {} };
    }

    auto [first_contains, first_target] = self(self, current->first.get());
    if (first_target.parent) {
      return { true, first_target };
    }

    auto [second_contains, second_target] = self(self, current->second.get());
    if (second_target.parent) {
      return { true, second_target };
    }

    auto contains = first_contains or second_contains;
    if (not contains) {
      return { false, {} };
    }

    auto first_child = first_contains;
    auto edge = ResizeEdge::None;
    if (current->kind == SplitKind::Horizontal) {
      edge = first_child ? ResizeEdge::Right : ResizeEdge::Left;
    } else if (current->kind == SplitKind::Vertical) {
      edge = first_child ? ResizeEdge::Bottom : ResizeEdge::Top;
    }

    if (edge == desired_edge) {
      return { true, { current, first_child, edge } };
    }
    return { true, {} };
  };

  return find_impl(find_impl, node).second;
}

auto DockSpace::layout_node(DockNode* node, Rect rect) -> void {
  if (not node) {
    return;
  }
  node->bounds = rect;

  if (node->kind == SplitKind::Leaf) {
    if (node->panel) {
      node->panel->bounds = with_dock_gap(rect);
    }
    return;
  }

  if (node->kind == SplitKind::Horizontal) {
    auto first_w = std::max(MinPanelWidth, rect.size.x * node->split);
    first_w = std::min(first_w, rect.size.x - MinPanelWidth);
    layout_node(node->first.get(), { rect.position, { first_w, rect.size.y } });
    layout_node(node->second.get(),
                { { rect.position.x + first_w, rect.position.y },
                  { rect.size.x - first_w, rect.size.y } });
    return;
  }

  auto first_h = std::max(MinPanelHeight, rect.size.y * node->split);
  first_h = std::min(first_h, rect.size.y - MinPanelHeight);
  layout_node(node->first.get(), { rect.position, { rect.size.x, first_h } });
  layout_node(node->second.get(),
              { { rect.position.x, rect.position.y + first_h },
                { rect.size.x, rect.size.y - first_h } });
}

auto DockSpace::remove_from_tree(DockPanel* panel) -> void {
  auto remove_impl = [&](auto&& self, UniquePtr<DockNode>& node) -> bool {
    if (not node) {
      return false;
    }
    if (node->kind == SplitKind::Leaf) {
      if (node->panel != panel) {
        return false;
      }
      node = nullptr;
      return true;
    }

    if (self(self, node->first)) {
      if (not node->first and node->second) {
        node = std::move(node->second);
      }
      return true;
    }
    if (self(self, node->second)) {
      if (not node->second and node->first) {
        node = std::move(node->first);
      }
      return true;
    }
    return false;
  };

  remove_impl(remove_impl, m_root);
}

auto DockSpace::dock_panel(DockPanel* panel, DockNode* target, DockSlot slot)
    -> void {
  if (not panel) {
    return;
  }
  remove_from_tree(panel);

  auto new_leaf = unique<DockNode>();
  new_leaf->panel = panel;

  if (not m_root or not target) {
    m_root = std::move(new_leaf);
    panel->slot = DockSlot::Center;
    return;
  }

  while (target and target->kind != SplitKind::Leaf) {
    target = target->second ? target->second.get() : target->first.get();
  }
  if (not target) {
    m_root = std::move(new_leaf);
    panel->slot = DockSlot::Center;
    return;
  }

  auto existing = unique<DockNode>();
  existing->kind = target->kind;
  existing->panel = target->panel;
  existing->first = std::move(target->first);
  existing->second = std::move(target->second);
  existing->split = target->split;
  existing->bounds = target->bounds;

  target->panel = nullptr;
  target->split = 0.5f;
  target->first = nullptr;
  target->second = nullptr;

  switch (slot) {
    case DockSlot::Left:
      target->kind = SplitKind::Horizontal;
      target->first = std::move(new_leaf);
      target->second = std::move(existing);
      break;
    case DockSlot::Right:
      target->kind = SplitKind::Horizontal;
      target->first = std::move(existing);
      target->second = std::move(new_leaf);
      break;
    case DockSlot::Top:
      target->kind = SplitKind::Vertical;
      target->first = std::move(new_leaf);
      target->second = std::move(existing);
      break;
    case DockSlot::Bottom:
      target->kind = SplitKind::Vertical;
      target->first = std::move(existing);
      target->second = std::move(new_leaf);
      break;
    case DockSlot::Center:
    case DockSlot::Floating:
      target->kind = SplitKind::Leaf;
      target->panel = panel;
      break;
  }

  panel->slot = DockSlot::Center;
}

auto DockSpace::dock_slot_at(vec2 mouse) -> DockSlot {
  if (auto* node = find_node_at(m_root.get(), mouse)) {
    auto slot = dock_edge_at(node->bounds, mouse);
    m_drop_node = slot == DockSlot::Floating ? nullptr : node;
    return slot;
  }

  auto content = content_rect(bounds);
  auto slot = dock_edge_at(content, mouse);
  m_drop_node = slot == DockSlot::Floating ? nullptr : m_root.get();
  return slot;
}

auto DockSpace::dock_rect(DockSlot slot) const -> Rect {
  auto target = m_drop_node ? m_drop_node->bounds : content_rect(bounds);
  auto half_w = target.size.x * 0.5f;
  auto half_h = target.size.y * 0.5f;

  switch (slot) {
    case DockSlot::Left:
      return with_dock_gap({ target.position, { half_w, target.size.y } });
    case DockSlot::Right:
      return with_dock_gap({ { target.position.x + half_w, target.position.y },
                             { target.size.x - half_w, target.size.y } });
    case DockSlot::Top:
      return with_dock_gap({ target.position, { target.size.x, half_h } });
    case DockSlot::Bottom:
      return with_dock_gap({ { target.position.x, target.position.y + half_h },
                             { target.size.x, target.size.y - half_h } });
    case DockSlot::Center:
      return with_dock_gap(target);
    case DockSlot::Floating:
      break;
  }

  auto content = content_rect(bounds);
  return { content.position + vec2 { 80.0f, 80.0f }, { 360.0f, 260.0f } };
}

} // namespace zod::sodium::custom

///////////////////////////////////////////////////////////
namespace {}
