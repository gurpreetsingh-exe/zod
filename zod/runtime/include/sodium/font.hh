#pragma once

#include "gpu/fwd.hh"

namespace zod::sodium {

auto init_font(const fs::path&) -> void;

class Font {
public:
  Font();
  ~Font();

public:
  static auto get() -> Font&;
  auto load_font(const fs::path&) -> void;
  auto get_texture_atlas() -> void*;
  auto get_width() -> i32 { return m_width; }
  auto get_height() -> i32 { return m_height; }
  auto render_text(const char* text, f32 x, f32 y, f32 sx, f32 sy) -> void;
  auto render_text_center(const char* text, f32 x, f32 y, f32 size) -> void;
  auto width(const String&, f32) -> f32;
  auto submit() -> void;

private:
  struct GlyphInfo {
    f32 advance = 0.0f;
    f32 plane_left = 0.0f;
    f32 plane_bottom = 0.0f;
    f32 plane_right = 0.0f;
    f32 plane_top = 0.0f;
    f32 uv_left = 0.0f;
    f32 uv_top = 0.0f;
    f32 uv_right = 0.0f;
    f32 uv_bottom = 0.0f;
    bool visible = false;
  };

private:
  GlyphInfo m_glyphs[128] = {};
  f32 m_kerning[128][128] = {};
  i32 m_width = 0;
  i32 m_height = 0;
  SharedPtr<GPUTexture> m_texture;
  SharedPtr<GPUBatch> m_batch;
  SharedPtr<GPUShader> m_text_shader;
  f32* m_position = nullptr;
  f32* m_uv = nullptr;
  usize m_coord_count = 0;
  usize m_nvert = 0;
};

} // namespace zod::sodium
