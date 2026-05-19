#pragma once

#include "core/option.hh"
#include "core/shapes.hh"

#include "gpu/batch.hh"
#include "gpu/shader.hh"
#include "gpu/texture.hh"

#include "application/event.hh"

namespace zod {
class GPUTexture;
} // namespace zod

namespace zod::sodium {

auto init_font(const fs::path&) -> void;

class Font {
public:
  Font();
  ~Font();

public:
  static auto get() -> Font&;
  auto load_font(const fs::path&) -> void;
  auto get_texture_atlas() -> void* { return m_texture->get_id(); }
  auto get_width() -> i32 { return m_width; }
  auto get_height() -> i32 { return m_height; }
  auto render_text(const char* text, f32 x, f32 y, f32 sx, f32 sy) -> void;
  auto render_text_center(const char* text, f32 x, f32 y, f32 size) -> void;
  auto width(const String&, f32) -> f32;
  auto submit() -> void;

private:
  struct GlyphInfo {
    f32 advance = 0.0f;
    f32 plane_left = 0.0f;
    f32 plane_bottom = 0.0f;
    f32 plane_right = 0.0f;
    f32 plane_top = 0.0f;
    f32 uv_left = 0.0f;
    f32 uv_top = 0.0f;
    f32 uv_right = 0.0f;
    f32 uv_bottom = 0.0f;
    bool visible = false;
  };

private:
  GlyphInfo m_glyphs[128] = {};
  f32 m_kerning[128][128] = {};
  i32 m_width = 0;
  i32 m_height = 0;
  SharedPtr<GPUTexture> m_texture;
  SharedPtr<GPUBatch> m_batch;
  SharedPtr<GPUShader> m_text_shader;
  f32* m_position = nullptr;
  f32* m_uv = nullptr;
  usize m_coord_count = 0;
  usize m_nvert = 0;
};

struct GPUDrawData {
  vec2 point;
  vec2 uv;
};

struct DrawList {
  Vector<GPUDrawData> draw_data = {};
  Vector<vec4> colors = {};
  Vector<u32> indices = {};
};

struct PaintCx {
  DrawList draw_list = {};
};

enum class IconId : int {
  None = -1,
  Close = 0,
  Maximize = 1,
  Minimize = 2,
};

enum RoundingFlags {
  RECT_BEVEL_NONE = 0,

  RECT_BEVEL_TOP_LEFT = 1 << 0,
  RECT_BEVEL_TOP_RIGHT = 1 << 1,
  RECT_BEVEL_BOTTOM_RIGHT = 1 << 2,
  RECT_BEVEL_BOTTOM_LEFT = 1 << 3,

  RECT_BEVEL_TOP = RECT_BEVEL_TOP_LEFT | RECT_BEVEL_TOP_RIGHT,
  RECT_BEVEL_BOTTOM = RECT_BEVEL_BOTTOM_LEFT | RECT_BEVEL_BOTTOM_LEFT,
  RECT_BEVEL_RIGHT = RECT_BEVEL_TOP_RIGHT | RECT_BEVEL_BOTTOM_RIGHT,
  RECT_BEVEL_LEFT = RECT_BEVEL_TOP_LEFT | RECT_BEVEL_BOTTOM_LEFT,

  RECT_BEVEL_ALL = 0xF,
};

struct rect {
  Rect inner;

  vec4 color = { 0.1f, 0.8f, 0.1f, 1 };
  f32 rounding = 0;
  f32 padding = 0;
  RoundingFlags flags = RECT_BEVEL_ALL;

  auto intersect(vec2) const -> bool;
  auto paint(PaintCx&) const -> void;
  auto operator->() -> Rect*;
  auto operator*() -> Rect&;
};

enum Axis : int {
  Horizontal,
  Vertical,
};

auto operator~(Axis) -> Axis;

enum class SizeRule {
  Auto,
  Fixed,
  Stretch,
};

enum class Align {
  Start,
  Center,
  End,
  Stretch,
};

enum class Dock {
  Left,
  Right,
  Top,
  Bottom,
  Fill,
  Tab,
};

enum class Visibility {
  Visible,
  Hidden,
  Collapsed,
  SelfHitTestInvisible,
};

struct WidgetStyle {
  Padding padding {};
  f32 gap = 0.0f;
  vec4 background { 0.0f, 0.0f, 0.0f, 0.0f };
  vec4 border_color { 0.0f, 0.0f, 0.0f, 0.0f };
  f32 border_thickness = 0.0f;
};

struct SlotStyle {
  Dock dock = Dock::Fill;
  Padding padding {};
  SizeRule horizontal_rule = SizeRule::Stretch;
  SizeRule vertical_rule = SizeRule::Stretch;
  vec2 size = {};
  f32 stretch_weight = 1.0f;
  Align horizontal_alignment = Align::Stretch;
  Align vertical_alignment = Align::Stretch;
};

struct DrawRect {
  Rect frame {};
  vec4 color { 0.0f, 0.0f, 0.0f, 0.0f };
};

class SlotBuilder;
class Widget;

class Children {
public:
  explicit Children(Widget* owner = nullptr) : m_owner(owner) {}
  virtual ~Children() = default;

  auto owner() const -> Widget* { return m_owner; }
  virtual auto size() const -> usize = 0;
  virtual auto at(usize index) const -> Widget* = 0;

protected:
  Widget* m_owner = nullptr;
};

class Widget {
public:
  virtual ~Widget() = default;

  auto name() const -> const String& { return m_name; }
  auto set_name(String name) -> void { m_name = std::move(name); }

  auto frame() const -> const Rect& { return m_frame; }
  auto desired_size() const -> const vec2& { return m_desired_size; }
  auto style() const -> const WidgetStyle& { return m_style; }
  auto style() -> WidgetStyle& { return m_style; }
  auto visibility() const -> Visibility { return m_visibility; }
  auto hovered() const -> bool { return m_hovered; }
  auto set_visibility(Visibility visibility) -> void {
    m_visibility = visibility;
  }
  auto set_min_size(vec2 size) -> void { m_min_size = size; }

  virtual auto event(const Event&) -> bool;
  virtual auto set_desired_size(vec2 size) -> void { m_desired_size = size; }
  virtual auto compute_desired_size(vec2) -> vec2 = 0;
  virtual auto arrange(const Rect&) -> void = 0;
  virtual auto paint(PaintCx&) const -> void = 0;
  virtual auto get_children() const -> const Children* { return nullptr; }

protected:
  auto cache_desired_size(vec2 desired) const -> vec2 {
    m_desired_size = desired;
    return m_desired_size;
  }

  auto push_self_draws(PaintCx&) const -> void;

  String m_name;
  WidgetStyle m_style {};
  Rect m_frame {};
  mutable vec2 m_desired_size {};
  mutable vec2 m_min_size {};
  Visibility m_visibility = Visibility::Visible;
  bool m_hovered = false;
};

struct SingleChildSlot {
  SharedPtr<Widget> child;
  SlotStyle style {};
};

class SingleSlotChild : public Children {
public:
  SingleSlotChild(Widget* owner, const SingleChildSlot* slot)
      : Children(owner), m_slot(slot) {}

  auto size() const -> usize override {
    if (m_slot and m_slot->child) {
      return 1u;
    }
    return 0u;
  }

  auto at(usize index) const -> Widget* override {
    if (index == 0 and m_slot and m_slot->child) {
      return m_slot->child.get();
    }
    return nullptr;
  }

private:
  const SingleChildSlot* m_slot = nullptr;
};

class CompoundWidget : public Widget {
public:
  CompoundWidget();

  auto set_child(SharedPtr<Widget> child, const SlotStyle& style = {})
      -> SingleChildSlot&;

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;
  auto get_children() const -> const Children* override {
    return m_children_view.get();
  }

protected:
  SingleChildSlot m_child_slot;
  SharedPtr<SingleSlotChild> m_children_view;
};

class Box : public CompoundWidget {
public:
  Box() = default;
  explicit Box(String name) { set_name(std::move(name)); }

  auto compute_desired_size(vec2) -> vec2 override;
};

class Image : public Widget {
public:
  Image() = default;
  explicit Image(SharedPtr<GPUTexture> image) : m_image(image) {}
  explicit Image(vec4 color) { set_image(color); }

  auto set_image(vec4 color) -> void { m_style.background = color; }
  auto set_accessible_text(String text) -> void {
    m_accessible_text = std::move(text);
  }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;

private:
  String m_accessible_text;
  SharedPtr<GPUTexture> m_image = nullptr;
};

class Button : public CompoundWidget {
public:
  struct Style {
    vec4 hovered_tint;
  };

  Button() = default;
  Button(IconId icon) : m_icon(icon) {}

  auto set_focusable(bool focusable) -> void { m_focusable = focusable; }
  auto set_enabled(bool enabled) -> void { m_enabled = enabled; }
  auto set_button_style(Style style) -> void { m_button_style = style; }
  auto set_content_padding(Padding padding) -> void {
    m_style.padding = padding;
  }

  auto compute_desired_size(vec2) -> vec2 override;
  auto paint(PaintCx&) const -> void override;

private:
  bool m_focusable = true;
  bool m_enabled = true;
  IconId m_icon = IconId::None;
  Style m_button_style = {};
};

class Menu : public Widget {
public:
  Menu() = default;

  auto compute_desired_size(vec2) -> vec2 override;
  auto paint(PaintCx&) const -> void override;
  auto arrange(const Rect&) -> void override;

private:
  bool m_open = false;
  Vector<SharedPtr<Widget>> m_buttons;
};

class Container : public Widget {
public:
  Container();

  struct Slot {
    SharedPtr<Widget> child;
    SlotStyle style {};
  };

  auto add_child(SharedPtr<Widget> child, const SlotStyle& style = {}) -> Slot&;
  auto child_count() const -> usize { return m_children.size(); }
  auto get_children() const -> const Children* override {
    return m_children_view.get();
  }

protected:
  Vector<Slot> m_children;
  SharedPtr<Children> m_children_view;
};

template <class SlotT>
class ContainerChild : public Children {
public:
  ContainerChild(Widget* owner, const Vector<SlotT>* slots)
      : Children(owner), m_slots(slots) {}

  auto size() const -> usize override {
    if (m_slots) {
      return m_slots->size();
    }
    return 0u;
  }

  auto at(usize index) const -> Widget* override {
    if (not m_slots or index >= m_slots->size()) {
      return nullptr;
    }
    return (*m_slots)[index].child.get();
  }

private:
  const Vector<SlotT>* m_slots = nullptr;
};

using ContainerChildren = ContainerChild<Container::Slot>;

class BoxContainer : public Container {
public:
  explicit BoxContainer(Axis axis = Axis::Vertical) : m_axis(axis) {}

  auto set_axis(Axis axis) -> void { m_axis = axis; }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;

private:
  Axis m_axis = Axis::Vertical;
};

class Stack : public BoxContainer {
public:
  explicit Stack(Axis axis = Axis::Vertical) : BoxContainer(axis) {}
};

class HorizontalBox : public BoxContainer {
public:
  HorizontalBox() : BoxContainer(Axis::Horizontal) {}

  static auto slot() -> SlotBuilder;
};

class VerticalBox : public BoxContainer {
public:
  VerticalBox() : BoxContainer(Axis::Vertical) {}

  static auto slot() -> SlotBuilder;
};

class DockArea : public Container {
public:
  auto set_active_tab(usize index) -> void { m_active_tab = index; }
  auto active_tab() const -> Option<usize> { return m_active_tab; }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;

private:
  Option<usize> m_active_tab;
};

class Border : public CompoundWidget {
public:
  auto paint(PaintCx&) const -> void override;
};

class TitleBar : public CompoundWidget {
public:
  TitleBar();
};

class DPIScaler : public CompoundWidget {
public:
  explicit DPIScaler(f32 scale = 1.0f) : m_scale(scale) {}

  auto set_scale(f32 scale) -> void { m_scale = scale; }
  auto scale() const -> f32 { return m_scale; }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;

private:
  f32 m_scale = 1.0f;
};

class Overlay : public Container {
public:
  Overlay();

  struct OverlaySlot {
    SharedPtr<Widget> child;
    SlotStyle style {};
    int layer = 0;
  };

  auto add_overlay_child(SharedPtr<Widget> child, const SlotStyle& style = {},
                         int layer = 0) -> OverlaySlot&;

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;
  auto get_children() const -> const Children* override {
    return m_overlay_children_view.get();
  }

private:
  Vector<OverlaySlot> m_overlay_children;
  SharedPtr<ContainerChild<OverlaySlot>> m_overlay_children_view;
};

template <class WidgetT>
class WidgetBuilder;

struct SlotContent {
  SharedPtr<Widget> child;
  SlotStyle style {};
  int layer = 0;
};

class SlotBuilder {
public:
  auto dock(Dock dock) -> SlotBuilder& {
    m_style.dock = dock;
    return *this;
  }

  auto padding(Padding padding) -> SlotBuilder& {
    m_style.padding = padding;
    return *this;
  }

  auto padding(f32 value) -> SlotBuilder& {
    m_style.padding = value;
    return *this;
  }

  auto auto_width() -> SlotBuilder& {
    m_style.horizontal_rule = SizeRule::Auto;
    m_style.size.x = 0.0f;
    return *this;
  }

  auto auto_height() -> SlotBuilder& {
    m_style.vertical_rule = SizeRule::Auto;
    m_style.size.y = 0.0f;
    return *this;
  }

  auto fixed_width(f32 horizontal_size) -> SlotBuilder& {
    m_style.horizontal_rule = SizeRule::Fixed;
    m_style.size.x = horizontal_size;
    return *this;
  }

  auto fixed_height(f32 vertical_size) -> SlotBuilder& {
    m_style.vertical_rule = SizeRule::Fixed;
    m_style.size.y = vertical_size;
    return *this;
  }

  auto stretch_width(f32 weight = 1.0f) -> SlotBuilder& {
    m_style.horizontal_rule = SizeRule::Stretch;
    m_style.stretch_weight = weight;
    return *this;
  }

  auto stretch_height(f32 weight = 1.0f) -> SlotBuilder& {
    m_style.vertical_rule = SizeRule::Stretch;
    m_style.stretch_weight = weight;
    return *this;
  }

  auto halign(Align align) -> SlotBuilder& {
    m_style.horizontal_alignment = align;
    return *this;
  }

  auto valign(Align align) -> SlotBuilder& {
    m_style.vertical_alignment = align;
    return *this;
  }

  auto layer(int layer) -> SlotBuilder& {
    m_layer = layer;
    return *this;
  }

  auto build() const -> SlotStyle { return m_style; }
  operator SlotStyle() const { return m_style; }

  auto operator[](SharedPtr<Widget> child) const -> SlotContent {
    return { child, m_style, m_layer };
  }

  template <class WidgetT>
  auto operator[](WidgetBuilder<WidgetT>&& child) const -> SlotContent;

  template <class WidgetT>
  auto operator[](WidgetBuilder<WidgetT>& child) const -> SlotContent;

private:
  SlotStyle m_style {};
  int m_layer = 0;
};

inline auto slot() -> SlotBuilder { return {}; }

template <class WidgetT>
class WidgetBuilder {
public:
  template <class... Args>
  explicit WidgetBuilder(Args&&... args)
      : m_widget(shared<WidgetT>(std::forward<Args>(args)...)) {}

  auto name(String name) -> WidgetBuilder& {
    m_widget->set_name(std::move(name));
    return *this;
  }

  auto padding(Padding padding) -> WidgetBuilder& {
    m_widget->style().padding = padding;
    return *this;
  }

  auto padding(f32 value) -> WidgetBuilder& {
    m_widget->style().padding = Padding { value };
    return *this;
  }

  auto gap(f32 gap) -> WidgetBuilder& {
    m_widget->style().gap = gap;
    return *this;
  }

  auto background(vec4 color) -> WidgetBuilder& {
    m_widget->style().background = color;
    return *this;
  }

  auto border(vec4 color, f32 thickness = 1.0f) -> WidgetBuilder& {
    m_widget->style().border_color = color;
    m_widget->style().border_thickness = thickness;
    return *this;
  }

  auto desired_size(vec2 size) -> WidgetBuilder& {
    m_widget->set_desired_size(size);
    return *this;
  }

  auto min_size(vec2 size) -> WidgetBuilder& {
    m_widget->set_min_size(size);
    return *this;
  }

  auto image(vec4 color) -> WidgetBuilder&
    requires std::is_base_of_v<Image, WidgetT>
  {
    m_widget->set_image(color);
    return *this;
  }

  auto accessible_text(String text) -> WidgetBuilder&
    requires std::is_base_of_v<Image, WidgetT>
  {
    m_widget->set_accessible_text(std::move(text));
    return *this;
  }

  auto visibility(Visibility visibility) -> WidgetBuilder& {
    m_widget->set_visibility(visibility);
    return *this;
  }

  auto is_focusable(bool focusable) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_focusable(focusable);
    return *this;
  }

  auto is_enabled(bool enabled) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_enabled(enabled);
    return *this;
  }

  auto content_padding(Padding padding) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_content_padding(padding);
    return *this;
  }

  auto content_padding(f32 value) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_content_padding(Padding { value });
    return *this;
  }

  auto button_style(const Button::Style& style) -> WidgetBuilder&
    requires std::is_base_of_v<Button, WidgetT>
  {
    m_widget->set_button_style(style);
    return *this;
  }

  auto active_tab(usize index) -> WidgetBuilder&
    requires std::is_base_of_v<DockArea, WidgetT>
  {
    m_widget->set_active_tab(index);
    return *this;
  }

  auto slot(SharedPtr<Widget> child, const SlotStyle& style = {})
      -> WidgetBuilder&
    requires std::is_base_of_v<Container, WidgetT>
  {
    if constexpr (std::is_base_of_v<Overlay, WidgetT>) {
      m_widget->add_overlay_child(child, style);
    } else {
      m_widget->add_child(child, style);
    }
    return *this;
  }

  template <class ChildT>
  auto slot(WidgetBuilder<ChildT>&& child, const SlotStyle& style = {})
      -> WidgetBuilder&
    requires std::is_base_of_v<Container, WidgetT>
  {
    return slot(child.build(), style);
  }

  auto child(SharedPtr<Widget> child, const SlotStyle& style = {})
      -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    m_widget->set_child(std::move(child), style);
    return *this;
  }

  template <class ChildT>
  auto child(WidgetBuilder<ChildT>&& child, const SlotStyle& style = {})
      -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    return child(child.build(), style);
  }

  auto operator+(SlotContent content) -> WidgetBuilder&
    requires std::is_base_of_v<Container, WidgetT>
  {
    if constexpr (std::is_base_of_v<Overlay, WidgetT>) {
      m_widget->add_overlay_child(std::move(content.child), content.style,
                                  content.layer);
    } else {
      m_widget->add_child(std::move(content.child), content.style);
    }
    return *this;
  }

  auto operator[](SharedPtr<Widget> child) -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    m_widget->set_child(std::move(child));
    return *this;
  }

  template <class ChildT>
  auto operator[](WidgetBuilder<ChildT>&& child) -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    return (*this)[child.build()];
  }

  template <class ChildT>
  auto operator[](WidgetBuilder<ChildT>& child) -> WidgetBuilder&
    requires std::is_base_of_v<CompoundWidget, WidgetT>
  {
    return (*this)[child.build()];
  }

  auto build() -> SharedPtr<WidgetT> { return m_widget; }
  auto operator*() -> SharedPtr<WidgetT> { build(); }
  operator SharedPtr<WidgetT>() { return build(); }

private:
  SharedPtr<WidgetT> m_widget;
};

template <class WidgetT>
auto SlotBuilder::operator[](WidgetBuilder<WidgetT>&& child) const
    -> SlotContent {
  return (*this)[child.build()];
}

template <class WidgetT>
auto SlotBuilder::operator[](WidgetBuilder<WidgetT>& child) const
    -> SlotContent {
  return (*this)[child.build()];
}

template <class WidgetT, class... Args>
auto create(Args&&... args) -> WidgetBuilder<WidgetT> {
  return WidgetBuilder<WidgetT>(std::forward<Args>(args)...);
}

auto init_gui() -> void;
auto submit(PaintCx&) -> void;
auto submit(PaintCx&, SharedPtr<GPUTexture>) -> void;
auto draw_images() -> void;
auto icon(IconId, Rect, vec4 color = { 0.86f, 0.90f, 0.95f, 1.0f }) -> void;
auto flush_icons() -> void;
auto present() -> void;

} // namespace zod::sodium
