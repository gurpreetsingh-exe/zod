#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "gpu/backend.hh"

namespace zod {

auto init_font(const fs::path&) -> void;

class Font {
public:
  static constexpr usize size = 20;

public:
  Font();
  ~Font();

public:
  static auto get() -> Font&;
  auto load_font(const fs::path&) -> void;
  auto get_texture_atlas() -> void* { return m_texture->get_id(); }
  auto get_width() -> i32 { return m_width; }
  auto get_height() -> i32 { return m_height; }
  auto render_text(const char* text, f32 x, f32 y, f32 sx, f32 sy) -> void;
  auto render_text_center(const char* text, f32 x, f32 y, f32 size) -> void;
  auto submit() -> void;

private:
  FT_Library m_ft;
  FT_Face m_face;
  i32 m_width;
  i32 m_height;
  SharedPtr<GPUTexture> m_texture;
  SharedPtr<GPUBatch> m_batch;
  SharedPtr<GPUShader> m_text_shader;
  f32* m_position = nullptr;
  f32* m_uv = nullptr;
  usize m_coord_count = 0;
  usize m_nvert = 0;
};

} // namespace zod
