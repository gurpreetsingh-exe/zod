#pragma once

#include "gpu/fwd.hh"
#include "sodium_v2/paint.hh"

namespace zod::sodium {

struct Cx {
  f32 padding = 4.0f;
};

extern Cx S;

auto init_gui() -> void;
auto resize(vec2) -> void;
auto begin_frame() -> void;
auto end_frame(PaintCx&) -> void;
auto submit(PaintCx&) -> void;
auto submit(PaintCx&, SharedPtr<GPUTexture>) -> void;
auto flush_foreground(PaintCx&) -> void;
auto draw_images() -> void;
auto present() -> void;

template <class DrawFn>
auto frame(DrawFn draw) -> void {
  begin_frame();

  auto cx = PaintCx();
  draw(cx);

  end_frame(cx);
}

} // namespace zod::sodium
