#pragma once

#include "core/shapes.hh"
#include "gpu/fwd.hh"

namespace zod::sodium {

enum RoundingFlags {
  RECT_BEVEL_NONE = 0,

  RECT_BEVEL_TOP_LEFT = 1 << 0,
  RECT_BEVEL_TOP_RIGHT = 1 << 1,
  RECT_BEVEL_BOTTOM_RIGHT = 1 << 2,
  RECT_BEVEL_BOTTOM_LEFT = 1 << 3,

  RECT_BEVEL_TOP = RECT_BEVEL_TOP_LEFT | RECT_BEVEL_TOP_RIGHT,
  RECT_BEVEL_BOTTOM = RECT_BEVEL_BOTTOM_LEFT | RECT_BEVEL_BOTTOM_LEFT,
  RECT_BEVEL_RIGHT = RECT_BEVEL_TOP_RIGHT | RECT_BEVEL_BOTTOM_RIGHT,
  RECT_BEVEL_LEFT = RECT_BEVEL_TOP_LEFT | RECT_BEVEL_BOTTOM_LEFT,

  RECT_BEVEL_ALL = 0xF,
};

struct GPUDrawData {
  vec2 point;
  vec2 uv;
};

struct DrawList {
  Vector<GPUDrawData> draw_data = {};
  Vector<vec4> colors = {};
  Vector<u32> indices = {};
};

struct PaintCx {
  DrawList draw_list = {};
};

struct rect {
  Rect inner;

  vec4 color = { 0.1f, 0.8f, 0.1f, 1 };
  f32 rounding = 0;
  f32 padding = 0;
  RoundingFlags flags = RECT_BEVEL_ALL;

  auto intersect(vec2) const -> bool;
  auto paint(PaintCx&) const -> void;
  auto operator->() -> Rect*;
  auto operator*() -> Rect&;
};

struct DrawData {
  SharedPtr<GPUBuffer> draw_data;
  SharedPtr<GPUBuffer> colors;
  SharedPtr<GPUTexture> white_texture;
  SharedPtr<GPUShader> shader;
  SharedPtr<GPUShader> present;
  SharedPtr<GPUBatch> batch;
  SharedPtr<GPUFrameBuffer> framebuffer;
  mat4 projection = mat4(1.0f);
};

extern DrawData* dcx;

} // namespace zod::sodium
