#pragma once

#include "widget.hh"

namespace zod {

class Panel : public Widget {
public:
  Panel() = default;
  auto calculate(f32 x, f32 y, f32 w, f32 h) -> void override {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }

  auto on_event(Event&) -> void override {}

  auto generate(std::vector<vec2>& offsets) -> void override {
    offsets.push_back({ x, y });
    offsets.push_back({ x + w, y });
    offsets.push_back({ x, y + h });
    offsets.push_back({ x + w, y + h });
  }
};

} // namespace zod
