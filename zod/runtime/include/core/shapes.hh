#pragma once

namespace zod {

enum class AnchorPoint : int {
  BottomLeft = 0,
  TopLeft,
};

struct Rect;

struct Rect0 {
  f32 x;
  f32 y;
  f32 w;
  f32 h;

  auto to_rect() const -> Rect;
};

struct Padding {
  f32 left = 0.0f;
  f32 right = 0.0f;
  f32 top = 0.0f;
  f32 bottom = 0.0f;

  Padding() = default;
  Padding(f32 left, f32 right, f32 top, f32 bottom)
      : left(left), right(right), top(top), bottom(bottom) {}
  Padding(f32 v) : Padding(v, v, v, v) {}
  auto combined() const -> vec2;
  auto horizontal() const -> f32;
  auto vertical() const -> f32;
};

struct Rect {
  /// point at the bottom left corner
  vec2 position;
  vec2 size;
  AnchorPoint anchor = AnchorPoint::BottomLeft;

  Rect() = default;
  Rect(vec2 /* position */, vec2 /* size */);
  Rect(f32 /* x */, f32 /* y */, f32 /* width */, f32 /* height */);
  auto intersect(vec2 /* point */) const -> bool;
  auto contains(const Rect&) const -> bool;

  auto padding(Padding /* padding */) const -> Rect;
  auto padding(vec2 /* padding */) const -> Rect;
  auto padding(f32 /* padding */) const -> Rect;

  auto margin(vec2 /* margin */) const -> Rect;
  auto margin(f32 /* margin */) const -> Rect;

  auto bbox() const -> Rect;
  operator vec4() const { return { position, size }; }
  operator Rect0() const { return { position.x, position.y, size.x, size.y }; }

  friend auto operator*(const Rect& r, f32 n) -> Rect {
    return { r.position * n, r.size * n };
  }
};

inline auto Rect0::to_rect() const -> Rect { return { { x, y }, { w, h } }; }

} // namespace zod
