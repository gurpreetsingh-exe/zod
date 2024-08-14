#pragma once

#include "base/math.hh"

namespace zod {

class GPURenderer {
public:
  virtual ~GPURenderer() = default;
  static auto get() -> GPURenderer&;
  virtual auto clear_color(const vec4&) -> void = 0;
  virtual auto resize(int /* width */, int /* height */) -> void = 0;
};

} // namespace zod
