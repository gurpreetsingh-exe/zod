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

  auto calculate(f32& x, f32& y, f32& w, f32& h) -> void override {
    ZASSERT(m_nodes.size() == m_size_coefficients.size());
    for (const auto& [node, size] :
         rng::views::zip(m_nodes, m_size_coefficients)) {
      switch (m_kind) {
        case SplitKind::Horizontal: {
          node->calculate(x, y, w, size);
          y += size;
        } break;
        case SplitKind::Vertical: {
          node->calculate(x, y, size, h);
          x += size;
        } break;
      }
    }
  }

  // auto generate(GMesh& mesh) -> void override {
  //   for (const auto& node : m_nodes) { node->generate(mesh); }
  // }

private:
  SplitKind m_kind;
  std::vector<Unique<Widget>> m_nodes;
  std::vector<f32> m_size_coefficients;
};

} // namespace zod
