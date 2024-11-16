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

  auto on_event(Event& event) -> void override {
    for (const auto& node : m_nodes) { node->on_event(event); }
  }

  auto draw() -> void override {
    for (const auto& node : m_nodes) { node->draw(); }
  }

private:
  SplitKind m_kind;
  std::vector<Unique<Widget>> m_nodes;
  std::vector<f32> m_size_coefficients;
};

} // namespace zod