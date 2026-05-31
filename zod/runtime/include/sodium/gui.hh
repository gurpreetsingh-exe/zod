#pragma once

#include "gpu/fwd.hh"
#include "sodium/paint.hh"

namespace zod::sodium {

struct Cx {
  f32 padding = 4.0f;
};

extern Cx S;

auto init_gui() -> void;
auto resize(vec2) -> void;
auto submit(PaintCx&) -> void;
auto submit(PaintCx&, SharedPtr<GPUTexture>) -> void;
auto draw_images() -> void;
auto present() -> void;

} // namespace zod::sodium
