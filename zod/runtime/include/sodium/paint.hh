#pragma once

#include "core/shapes.hh"
#include "gpu/fwd.hh"

#include <functional>

namespace zod::sodium {

enum RoundingFlags {
  RECT_ROUND_NONE = 0,

  RECT_ROUND_TOP_LEFT = 1 << 0,
  RECT_ROUND_TOP_RIGHT = 1 << 1,
  RECT_ROUND_BOTTOM_RIGHT = 1 << 2,
  RECT_ROUND_BOTTOM_LEFT = 1 << 3,

  RECT_ROUND_TOP = RECT_ROUND_TOP_LEFT | RECT_ROUND_TOP_RIGHT,
  RECT_ROUND_BOTTOM = RECT_ROUND_BOTTOM_LEFT | RECT_ROUND_BOTTOM_RIGHT,
  RECT_ROUND_RIGHT = RECT_ROUND_TOP_RIGHT | RECT_ROUND_BOTTOM_RIGHT,
  RECT_ROUND_LEFT = RECT_ROUND_TOP_LEFT | RECT_ROUND_BOTTOM_LEFT,

  RECT_ROUND_ALL = 0xF,
};

inline auto operator|(RoundingFlags a, RoundingFlags b) -> RoundingFlags {
  return RoundingFlags(int(a) | int(b));
}

inline auto operator&(RoundingFlags a, RoundingFlags b) -> RoundingFlags {
  return RoundingFlags(int(a) & int(b));
}

struct GPUDrawData {
  vec2 point;
  vec2 uv;
};

struct DrawList {
  Vector<GPUDrawData> draw_data = {};
  Vector<vec4> colors = {};
  Vector<u32> indices = {};
};

struct TextDrawList {
  Vector<f32> positions = {};
  Vector<f32> uvs = {};

  auto clear() -> void {
    positions.clear();
    uvs.clear();
  }
};

struct PaintCx {
  using DeferredPaint = std::function<void(PaintCx&)>;

  DrawList draw_list = {};
  DrawList foreground_draw_list = {};
  TextDrawList text_draw_list = {};
  TextDrawList foreground_text_draw_list = {};
  Vector<DeferredPaint> deferred_foreground_paints = {};
  bool paint_foreground = false;

  auto active_draw_list() -> DrawList& {
    return paint_foreground ? foreground_draw_list : draw_list;
  }

  auto active_text_draw_list() -> TextDrawList& {
    return paint_foreground ? foreground_text_draw_list : text_draw_list;
  }

  auto defer_foreground(DeferredPaint paint) -> void {
    deferred_foreground_paints.push_back(std::move(paint));
  }
};

class ScopedForegroundPaint {
public:
  explicit ScopedForegroundPaint(PaintCx& cx)
      : m_cx(cx), m_previous(cx.paint_foreground) {
    m_cx.paint_foreground = true;
  }

  ~ScopedForegroundPaint() { m_cx.paint_foreground = m_previous; }

private:
  PaintCx& m_cx;
  bool m_previous = false;
};

struct rect {
  Rect inner;

  vec4 color = { 0.1f, 0.8f, 0.1f, 1 };
  f32 rounding = 0;
  f32 padding = 0;
  RoundingFlags flags = RECT_ROUND_ALL;

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
