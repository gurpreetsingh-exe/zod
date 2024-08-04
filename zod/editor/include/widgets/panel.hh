#pragma once

#include "widget.hh"

namespace zod {

class Panel : public Widget {
public:
  Panel() = default;
  auto calculate(f32& x, f32& y, f32& w, f32& h) -> void override {
    // fmt::println("{}, {}, {}, {}", x, y, w, h);
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }

  // auto generate(GMesh& mesh) -> void override {
  //   x = (x - 0.5f) * 2.0f;
  //   y = (y - 0.5f) * 2.0f;
  //   w = w * 2.0f;
  //   h = h * 2.0f;
  //   // f32 px = m_padding * aspect;
  //   // f32 py = m_padding;
  //   f32 px = 0;
  //   f32 py = 0;
  //   /// top right
  //   mesh.points.push_back({ x + w - px, y + py });
  //   mesh.uvs.push_back({ 1, 0 });
  //   mesh.size.push_back({ w, h });
  //   /// bottom right
  //   mesh.points.push_back({ x + w - px, y + h - py });
  //   mesh.uvs.push_back({ 1, 1 });
  //   mesh.size.push_back({ w, h });
  //   /// top left
  //   mesh.points.push_back({ x + px, y + py });
  //   mesh.uvs.push_back({ 0, 0 });
  //   mesh.size.push_back({ w, h });

  //   /// bottom right
  //   mesh.points.push_back({ x + w - px, y + h - py });
  //   mesh.uvs.push_back({ 1, 1 });
  //   mesh.size.push_back({ w, h });
  //   /// bottom left
  //   mesh.points.push_back({ x + px, y + h - py });
  //   mesh.uvs.push_back({ 0, 1 });
  //   mesh.size.push_back({ w, h });
  //   /// top left
  //   mesh.points.push_back({ x + px, y + py });
  //   mesh.uvs.push_back({ 0, 0 });
  //   mesh.size.push_back({ w, h });
  // }

private:
  f32 x = 0;
  f32 y = 0;
  f32 w = 0;
  f32 h = 0;
  f32 m_padding = 0.005;
};

} // namespace zod
