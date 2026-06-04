#include "sodium_v2/widgets/menu.hh"
#include "sodium_v2/font.hh"
#include "sodium_v2/style.hh"
#include "sodium_v2/theme.hh"
#include "sodium_v2/widgets/box.hh"
#include "sodium_v2/widgets/box_container.hh"
#include "sodium_v2/widgets/builder.hh"
#include "sodium_v2/widgets/text.hh"

namespace zod::sodium {

static constexpr auto MenuItemHeight = 24.0f;
static constexpr auto MenuSeparatorHeight = 6.0f;
static constexpr auto MenuPopupWidth = 180.0f;
static constexpr auto MenuPadding = 8.0f;
static constexpr auto MenuBorderThickness = 1.0f;

class MenuStack {
public:
  static auto push(Menu* menu) -> void {
    if (not menu) {
      return;
    }

    auto existing_index = index_of(menu);
    if (existing_index >= 0) {
      dismiss_after(static_cast<usize>(existing_index));
    } else {
      dismiss_all();
      s_stack.push_back(menu);
    }

    menu->set_open_from_stack(true);
  }

  static auto dismiss(Menu* menu) -> void {
    auto index = index_of(menu);
    if (index < 0) {
      if (menu) {
        menu->set_open_from_stack(false);
      }
      return;
    }

    dismiss_from(static_cast<usize>(index));
  }

  static auto dismiss_all() -> void { dismiss_from(0); }

  static auto has_open_menus() -> bool { return not s_stack.empty(); }

  static auto contains(Menu* menu) -> bool { return index_of(menu) >= 0; }

  static auto remove(Menu* menu) -> void {
    auto index = index_of(menu);
    if (index < 0) {
      return;
    }

    s_stack.erase(s_stack.begin() + index);
  }

private:
  static auto index_of(Menu* menu) -> i64 {
    for (auto i = usize(0); i < s_stack.size(); ++i) {
      if (s_stack[i] == menu) {
        return static_cast<i64>(i);
      }
    }
    return -1;
  }

  static auto dismiss_after(usize index) -> void {
    if (index + 1 >= s_stack.size()) {
      return;
    }

    dismiss_from(index + 1);
  }

  static auto dismiss_from(usize index) -> void {
    if (index >= s_stack.size()) {
      return;
    }

    for (auto i = s_stack.size(); i > index; --i) {
      if (auto* menu = s_stack[i - 1]) {
        menu->set_open_from_stack(false);
      }
    }
    s_stack.resize(index);
  }

private:
  static Vector<Menu*> s_stack;
};

Vector<Menu*> MenuStack::s_stack = {};

class MenuDismissLayer : public Widget {
public:
  explicit MenuDismissLayer(Menu* owner) : m_owner(owner) {
    set_hit_test_priority(10);
  }

  auto on_mouse_down(const Event& event) -> EventResponse override {
    if (event.button == MouseButton::Left and m_owner) {
      m_owner->close();
      return EventResponse::handled();
    }
    return EventResponse::unhandled();
  }

  auto compute_desired_size(vec2) -> vec2 override {
    return cache_desired_size({});
  }

  auto arrange(const Rect&) -> void override {
    m_frame = Rect(-100000.0f, -100000.0f, 200000.0f, 200000.0f);
  }

  auto paint(PaintCx&) const -> void override {}

private:
  Menu* m_owner = nullptr;
};

static auto make_menu_entry(Menu* owner, String label, Menu::Action action)
    -> SharedPtr<Widget> {
  auto& theme = Theme::get();
  return *create<Button>()
              .hit_test_priority(20)
              .min_size({ MenuPopupWidth, MenuItemHeight })
              .button_style({ theme.primary })
              .background(theme.background)[create<Text>(label).padding(
                  { MenuPadding, MenuPadding, 0, 0 })]
              .on_clicked([owner, action](const Event&) {
                auto close_after_selection =
                    owner and owner->close_after_selection();
                if (close_after_selection) {
                  owner->close();
                }
                if (action) {
                  action();
                }
                return EventResponse::handled();
              });
}

static auto make_menu_separator() -> SharedPtr<Widget> {
  auto& theme = Theme::get();
  return *create<Box>()
              .min_size({ MenuPopupWidth, MenuSeparatorHeight })
              .visibility(Visibility::SelfHitTestInvisible)
              .padding({ MenuPadding, MenuPadding, 2,
                         2 })[create<Box>().background(theme.foreground)];
}

Menu::Menu() : m_dismiss_layer(shared<MenuDismissLayer>(this)) {
  set_hit_test_priority(15);

  auto& theme = Theme::get();
  m_popup_content = shared<VerticalBox>();
  m_popup_root = *create<Box>()
                      .background(theme.highlight)
                      .min_desired_width(
                          MenuPopupWidth)[create<Box>().padding(1).background(
                          theme.background)[m_popup_content]];
}

Menu::~Menu() { MenuStack::remove(this); }

auto Menu::add_entry(String label, Action action) -> Menu& {
  m_popup_content->add_child(make_menu_entry(this, label, std::move(action)),
                             slot().fixed_height(MenuItemHeight));
  invalidate_layout();
  return *this;
}

auto Menu::add_separator() -> Menu& {
  m_popup_content->add_child(make_menu_separator(),
                             slot().fixed_height(MenuSeparatorHeight));
  invalidate_layout();
  return *this;
}

auto Menu::clear_entries() -> void {
  m_popup_content->clear_children();
  invalidate_layout();
}

auto Menu::open() -> void { MenuStack::push(this); }

auto Menu::close() -> void { MenuStack::dismiss(this); }

auto Menu::toggle() -> void {
  if (m_open) {
    close();
  } else {
    open();
  }
}

auto Menu::on_mouse_down(const Event& event) -> EventResponse {
  if (event.button == MouseButton::Left and m_hovered) {
    toggle();
    return EventResponse::handled();
  }
  return EventResponse::unhandled();
}

auto Menu::on_mouse_enter(const Event&) -> void {
  if (MenuStack::has_open_menus() and not MenuStack::contains(this)) {
    open();
  }
}

auto Menu::on_key_down(const Event& event) -> EventResponse {
  if (event.is_key_down(Key::Escape) and MenuStack::has_open_menus()) {
    MenuStack::dismiss_all();
    return EventResponse::handled();
  }
  return EventResponse::unhandled();
}

auto Menu::compute_desired_size(vec2 available) -> vec2 {
  auto width = Font::get().width(m_name, FontSizeInMenu);
  auto desired =
      max(vec2(width, FontSizeInMenu) + m_style.padding.combined(), m_min_size);
  cache_desired_size(desired);

  if (m_open) {
    m_dismiss_layer->compute_desired_size(available);
    m_popup_size = m_popup_root->compute_desired_size(available);
  }

  return desired;
}

auto Menu::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }

  auto& theme = Theme::get();
  auto background = m_open      ? theme.primary
                    : m_hovered ? theme.highlight
                                : m_style.background;
  if (background[3] > 0.0f) {
    rect(m_frame, background).paint(cx);
  }

  auto w = Font::get().width(m_name, FontSizeInMenu);
  auto center = m_frame.position + m_frame.size * 0.5f;
  Font::get().render_text_center(cx, m_name.c_str(), center.x - w * 0.5f,
                                 center.y, FontSizeInMenu);

  if (m_open) {
    cx.defer_foreground([popup = m_popup_root](PaintCx& foreground_cx) {
      popup->paint(foreground_cx);
    });
  }
}

auto Menu::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  if (m_open) {
    arrange_popup();
  }
}

auto Menu::get_children() const -> WidgetChildren {
  if (not m_open) {
    return {};
  }

  auto children = WidgetChildren {};
  children.reserve(2);
  children.push_back(m_dismiss_layer);
  children.push_back(m_popup_root);
  return children;
}

auto Menu::dismiss_all() -> void { MenuStack::dismiss_all(); }

auto Menu::set_open_from_stack(bool open) -> void {
  if (m_open == open) {
    return;
  }

  m_open = open;
  invalidate_layout();
}

auto Menu::arrange_popup() -> void {
  m_dismiss_layer->arrange({});
  m_popup_size = m_popup_root->desired_size();
  m_popup_root->arrange(Rect(m_frame.position.x,
                             m_frame.position.y + m_frame.size.y,
                             m_popup_size.x, m_popup_size.y));
}

} // namespace zod::sodium
