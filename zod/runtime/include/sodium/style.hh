#pragma once

#include "core/shapes.hh"

namespace zod::sodium {

class Widget;

constexpr f32 FontSizeInMenu = 12;

enum Axis : int {
  Horizontal,
  Vertical,
};

inline auto operator~(Axis axis) -> Axis {
  if (axis == Axis::Horizontal) {
    return Axis::Vertical;
  }
  return Axis::Horizontal;
}

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

enum class Visibility {
  Visible,
  Hidden,
  Collapsed,
  SelfHitTestInvisible,
};

struct SlotStyle {
  Padding padding {};
  SizeRule horizontal_rule = SizeRule::Stretch;
  SizeRule vertical_rule = SizeRule::Stretch;
  vec2 size = {};
  f32 horizontal_stretch_weight = 1.0f;
  f32 vertical_stretch_weight = 1.0f;
  Align horizontal_alignment = Align::Stretch;
  Align vertical_alignment = Align::Stretch;
};

struct SlotContent {
  SharedPtr<Widget> child;
  SlotStyle style {};
  int layer = 0;
};

template <class WidgetT>
class WidgetBuilder;

class SlotBuilder {
public:
  SlotBuilder() = default;
  SlotBuilder(SlotStyle style) : m_style(style) {}

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
    m_style.horizontal_stretch_weight = weight;
    return *this;
  }

  auto stretch_height(f32 weight = 1.0f) -> SlotBuilder& {
    m_style.vertical_rule = SizeRule::Stretch;
    m_style.vertical_stretch_weight = weight;
    return *this;
  }

  auto fill_width(f32 weight = 1.0f) -> SlotBuilder& {
    return stretch_width(weight);
  }

  auto fill_height(f32 weight = 1.0f) -> SlotBuilder& {
    return stretch_height(weight);
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

inline auto slot(SlotStyle style = {}) -> SlotBuilder { return { style }; }

template <class WidgetT>
auto SlotBuilder::operator[](WidgetBuilder<WidgetT>&& child) const
    -> SlotContent {
  return (*this)[*child];
}

template <class WidgetT>
auto SlotBuilder::operator[](WidgetBuilder<WidgetT>& child) const
    -> SlotContent {
  return (*this)[*child];
}

} // namespace zod::sodium
