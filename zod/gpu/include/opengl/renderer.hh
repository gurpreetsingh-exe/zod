#pragma once

#include "../renderer.hh"

namespace zod {

class GLRenderer : public GPURenderer {
public:
  auto clear_color(const vec4&) -> void override;
  auto resize(int w, int h) -> void override;
};

} // namespace zod
