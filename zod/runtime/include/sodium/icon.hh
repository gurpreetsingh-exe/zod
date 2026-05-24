#pragma once

#include "core/shapes.hh"

namespace zod::sodium {

enum class IconId : int {
  None = -1,
  Close = 0,
  Maximize = 1,
  Minimize = 2,
};

auto icon(IconId, Rect, vec4 color = { 0.86f, 0.90f, 0.95f, 1.0f }) -> void;
auto flush_icons() -> void;

} // namespace zod::sodium
