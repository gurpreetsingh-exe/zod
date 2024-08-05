#pragma once

#include "widget.hh"

namespace zod {

class Panel : public Widget {
public:
  Panel() = default;
  auto calculate(f32 x, f32 y, f32 w, f32 h) -> void override {
    // fmt::println("{}, {}, {}, {}", x, y, w, h);
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }

  auto generate(std::vector<vec2>& offsets) -> void override {
    offsets.push_back({ x, y });
    offsets.push_back({ x + w, y });
    offsets.push_back({ x, y + h });
    offsets.push_back({ x + w, y + h });
  }
};

} // namespace zod
