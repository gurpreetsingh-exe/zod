#pragma once

#include "widget.hh"

namespace zod {

enum class SplitKind {
  /// ┌───────┐
  /// │       │
  /// ├───────┤
  /// │       │
  /// └───────┘
  Horizontal,

  /// ┌───┬───┐
  /// │   │   │
  /// │   │   │
  /// │   │   │
  /// └───┴───┘
  Vertical,
};

class Split : public Widget {
public:
  Split(SplitKind kind)
      : m_kind(kind), m_nodes(std::vector<Unique<Widget>>()) {}

public:
  auto add_node(Unique<Widget> widget, f32 size) -> void {
    m_nodes.push_back(std::move(widget));
    m_size_coefficients.push_back(size);
  }

  auto calculate(f32 x, f32 y, f32 w, f32 h) -> void override {
    ZASSERT(m_nodes.size() == m_size_coefficients.size());
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    for (const auto& [node, size] :
         rng::views::zip(m_nodes, m_size_coefficients)) {
      switch (m_kind) {
        case SplitKind::Horizontal: {
          node->calculate(x, y, w, h * size);
          y += h * size;
        } break;
        case SplitKind::Vertical: {
          node->calculate(x, y, w * size, h);
          x += w * size;
        } break;
      }
    }
  }

  auto on_event(Event& event) -> void override {
    for (const auto& node : m_nodes) { node->on_event(event); }
  }

  auto get_widget(f32 x, f32 y) -> Widget* override {
    for (const auto& node : m_nodes) {
      if (node->x <= x and node->y <= y and node->x + node->w > x and
          node->y + node->h > y) {
        return node->get_widget(x, y);
      }
    }
    return this;
  }

  auto draw() -> void override {
    for (const auto& node : m_nodes) { node->draw(); }
  }

  auto generate(std::vector<vec2>& offsets) -> void override {
    for (const auto& node : m_nodes) { node->generate(offsets); }
  }

private:
  SplitKind m_kind;
  std::vector<Unique<Widget>> m_nodes;
  std::vector<f32> m_size_coefficients;
};

} // namespace zod
