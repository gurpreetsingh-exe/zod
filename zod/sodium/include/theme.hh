#pragma once

namespace zod {

struct Theme {
  vec4 background;
  vec4 highlight;
  vec4 primary;

  static auto load() -> void;
  static auto get() -> Theme&;
};

auto init_theme() -> void;

} // namespace zod
