#include "core/shapes.hh"

namespace zod {

using IntersectFn = bool (*)(vec2, vec2, vec2);

Vector<IntersectFn> intersect_functions = {
  [](vec2 position, vec2 size, vec2 point) -> bool {
    const auto bb = position + size;
    return point.x > position.x and point.y > position.y and point.x < bb.x and
           point.y < bb.y;
  },
};

auto Padding::combined() const -> vec2 { return { horizontal(), vertical() }; }
auto Padding::horizontal() const -> f32 { return left + right; }
auto Padding::vertical() const -> f32 { return top + bottom; }

Rect::Rect(vec2 position, vec2 size) : position(position), size(size) {}

Rect::Rect(f32 x, f32 y, f32 width, f32 height)
    : Rect({ x, y }, { width, height }) {}

auto Rect::intersect(vec2 point) const -> bool {
  const auto bb = position + size;
  return point.x > position.x and point.y > position.y and point.x < bb.x and
         point.y < bb.y;
}

auto Rect::padding(Padding p) const -> Rect {
  return { { position.x + p.left, position.y + p.top }, size - p.combined() };
}

auto Rect::padding(vec2 p) const -> Rect {
  return { position + p, size - p * 2.0f };
}

auto Rect::padding(f32 p) const -> Rect { return padding({ p, p }); }

auto Rect::margin(vec2 m) const -> Rect {
  return { position - m, size + m * 2.0f };
}

auto Rect::margin(f32 m) const -> Rect { return margin({ m, m }); }

auto Rect::contains(const Rect& r) const -> bool {
  const auto bb0 = position + size;
  const auto bb1 = r.position + r.size;

  return r.position.x >= position.x and bb1.x <= bb0.x and
         r.position.y >= position.y and bb1.y <= bb0.y;
}

} // namespace zod
