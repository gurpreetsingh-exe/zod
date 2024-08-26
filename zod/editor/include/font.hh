#pragma once

#include <glad/glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "backend.hh"

namespace zod {

class Font {
public:
  Font();

public:
  auto load_font(const fs::path&) -> void;
  auto get_texture_atlas() -> void* { return (void*)(intptr_t)m_texture; }
  auto get_width() -> i32 { return m_width; }
  auto get_height() -> i32 { return m_height; }
  auto render_text(const char* text, f32 x, f32 y, f32 sx, f32 sy) -> void;

private:
  FT_Library m_ft;
  FT_Face m_face;
  i32 m_width;
  i32 m_height;
  GLuint m_texture;
  Shared<GPUBatch> m_batch;
  Shared<GPUShader> m_text_shader;
  f32* m_position = nullptr;
  f32* m_uv = nullptr;
  usize m_coord_count = 0;
};

} // namespace zod
