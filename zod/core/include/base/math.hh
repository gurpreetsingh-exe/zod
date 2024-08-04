#pragma once

namespace zod {

struct vec2 {
  f32 x;
  f32 y;
};

struct vec4 {
  union {
    struct {
      f32 x, y, z, w;
    };
    struct {
      f32 r, g, b, a;
    };
  };
};

} // namespace zod
