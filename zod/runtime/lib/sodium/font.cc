#include "sodium/font.hh"
#include "gpu/backend.hh"
#include "sodium/paint.hh"

#include "shaders_generated.hh"

#include <msdfgen-ext.h>
#include <msdfgen.h>

namespace zod::sodium {

constexpr usize MAX_VERTICES = 64 * 1024;
constexpr i32 GlyphStart = 32;
constexpr i32 GlyphEnd = 128;
constexpr i32 GlyphAtlasWidth = 512;
constexpr i32 GlyphAtlasGap = 1;
constexpr i32 GlyphAtlasMaxGlyphSize = 128;
constexpr i32 GlyphAtlasPadding = 10;
constexpr f64 GlyphAtlasScale = 48.0;
constexpr f64 GlyphDistanceRange = 4.0 / GlyphAtlasScale;

static UniquePtr<Font> g_font = nullptr;

auto init_font(const fs::path& path) -> void {
  g_font = unique<Font>();
  g_font->load_font(path);
}

auto Font::get() -> Font& { return *g_font; }

auto Font::get_texture_atlas() -> void* { return m_texture->get_id(); }

struct PackedGlyph {
  i32 codepoint = 0;
  i32 width = 0;
  i32 height = 0;
  i32 atlas_x = 0;
  i32 atlas_y = 0;
  msdfgen::Shape::Bounds bounds = {};
  Vector<f32> pixels = {};
};

Font::Font() {
  m_position = new f32[MAX_VERTICES * 2];
  m_uv = new f32[MAX_VERTICES * 2];
  auto format = Vector<GPUBufferLayout> {
    { GPUDataType::Float, m_position, 2, MAX_VERTICES * 2 },
    { GPUDataType::Float, m_uv, 2, MAX_VERTICES * 2 },
  };
  auto indices = Vector<u32>();
  for (usize i = 0; i < MAX_VERTICES; i += 4) {
    indices.push_back(i);
    indices.push_back(i + 1);
    indices.push_back(i + 2);
    indices.push_back(i + 2);
    indices.push_back(i + 3);
    indices.push_back(i + 1);
  }

  m_batch = GPUBackend::get().create_batch(format, indices);

  m_text_shader =
      GPUBackend::get().create_shader(GPUShaderCreateInfo("text")
                                          .vertex_source(g_text_vert_src)
                                          .fragment_source(g_text_frag_src));
}

auto Font::load_font(const fs::path& path) -> void {
  auto* freetype = msdfgen::initializeFreetype();
  if (not freetype) {
    eprintln("Could not init freetype library");
    return;
  }

  auto* font = msdfgen::loadFont(freetype, path.string().c_str());
  if (not font) {
    eprintln("Could not open font {}", path.string());
    msdfgen::deinitializeFreetype(freetype);
    return;
  }

  auto packed_glyphs = Vector<PackedGlyph>();

  auto space_advance = 0.0;
  auto tab_advance = 0.0;
  msdfgen::getFontWhitespaceWidth(space_advance, tab_advance, font,
                                  msdfgen::FONT_SCALING_EM_NORMALIZED);

  for (auto codepoint = GlyphStart; codepoint < GlyphEnd; codepoint++) {
    auto shape = msdfgen::Shape();
    auto advance = 0.0;
    if (not msdfgen::loadGlyph(shape, font, msdfgen::unicode_t(codepoint),
                               msdfgen::FONT_SCALING_EM_NORMALIZED, &advance)) {
      continue;
    }

    auto& glyph = m_glyphs[codepoint];
    glyph.advance = f32(advance);
    if (codepoint == ' ' and space_advance > 0.0) {
      glyph.advance = f32(space_advance);
    } else if (codepoint == '\t' and tab_advance > 0.0) {
      glyph.advance = f32(tab_advance);
    }

    if (shape.edgeCount() == 0) {
      continue;
    }

    shape.normalize();
    auto bounds = shape.getBounds();
    auto draw_width = i32(std::ceil((bounds.r - bounds.l) * GlyphAtlasScale)) +
                      GlyphAtlasPadding * 2;
    auto draw_height = i32(std::ceil((bounds.t - bounds.b) * GlyphAtlasScale)) +
                       GlyphAtlasPadding * 2;
    draw_width = std::clamp(draw_width, 1, GlyphAtlasMaxGlyphSize);
    draw_height = std::clamp(draw_height, 1, GlyphAtlasMaxGlyphSize);

    msdfgen::edgeColoringSimple(shape, 3.0);
    auto msdf = msdfgen::Bitmap<f32, 3>(draw_width, draw_height);
    msdfgen::generateMSDF(
        msdf, shape, msdfgen::Range(GlyphDistanceRange),
        msdfgen::Vector2(GlyphAtlasScale, GlyphAtlasScale),
        msdfgen::Vector2(f64(GlyphAtlasPadding) / GlyphAtlasScale - bounds.l,
                         f64(GlyphAtlasPadding) / GlyphAtlasScale - bounds.b));

    auto& packed = packed_glyphs.emplace_back();
    packed.codepoint = codepoint;
    packed.width = draw_width;
    packed.height = draw_height;
    packed.bounds = bounds;
    packed.pixels.resize(usize(draw_width * draw_height * 3));
    for (int y = 0; y < draw_height; y++) {
      for (int x = 0; x < draw_width; x++) {
        auto* src = msdf(x, y);
        auto dst = usize(x + y * draw_width) * 3;
        packed.pixels[dst] = src[0];
        packed.pixels[dst + 1] = src[1];
        packed.pixels[dst + 2] = src[2];
      }
    }
  }

  std::sort(packed_glyphs.begin(), packed_glyphs.end(),
            [](const auto& a, const auto& b) {
              if (a.height == b.height) {
                return a.width > b.width;
              }
              return a.height > b.height;
            });

  auto pen_x = 0;
  auto pen_y = 0;
  auto row_height = 0;
  for (auto& packed : packed_glyphs) {
    if (pen_x > 0 and pen_x + packed.width > GlyphAtlasWidth) {
      pen_x = 0;
      pen_y += row_height + GlyphAtlasGap;
      row_height = 0;
    }

    packed.atlas_x = pen_x;
    packed.atlas_y = pen_y;
    pen_x += packed.width + GlyphAtlasGap;
    row_height = std::max(row_height, packed.height);
  }

  m_width = GlyphAtlasWidth;
  m_height = std::max(1, pen_y + row_height);
  auto pixels = Vector<f32>(m_width * m_height * 3, 0.0f);

  for (const auto& packed : packed_glyphs) {
    for (auto y = 0; y < packed.height; y++) {
      for (auto x = 0; x < packed.width; x++) {
        auto src = usize(x + y * packed.width) * 3;
        auto dst_x = packed.atlas_x + x;
        auto dst_y = packed.atlas_y + y;
        auto dst = usize(dst_x + dst_y * m_width) * 3;
        pixels[dst] = packed.pixels[src];
        pixels[dst + 1] = packed.pixels[src + 1];
        pixels[dst + 2] = packed.pixels[src + 2];
      }
    }

    auto& glyph = m_glyphs[packed.codepoint];
    auto left_pad = f32(f64(GlyphAtlasPadding) / GlyphAtlasScale);
    glyph.plane_left = f32(packed.bounds.l) - left_pad;
    glyph.plane_bottom = f32(packed.bounds.b) - left_pad;
    glyph.plane_right =
        glyph.plane_left + f32(f64(packed.width) / GlyphAtlasScale);
    glyph.plane_top =
        glyph.plane_bottom + f32(f64(packed.height) / GlyphAtlasScale);
    glyph.uv_left = f32(packed.atlas_x) / f32(m_width);
    glyph.uv_top = f32(packed.atlas_y + packed.height) / f32(m_height);
    glyph.uv_right = f32(packed.atlas_x + packed.width) / f32(m_width);
    glyph.uv_bottom = f32(packed.atlas_y) / f32(m_height);
    glyph.visible = true;
  }

  for (auto left = GlyphStart; left < GlyphEnd; left++) {
    for (auto right = GlyphStart; right < GlyphEnd; right++) {
      auto kerning = 0.0;
      if (msdfgen::getKerning(kerning, font, msdfgen::unicode_t(left),
                              msdfgen::unicode_t(right),
                              msdfgen::FONT_SCALING_EM_NORMALIZED)) {
        m_kerning[left][right] = f32(kerning);
      }
    }
  }

  m_texture = GPUBackend::get().create_texture({
      .name = "font-msdf-atlas",
      .width = m_width,
      .height = m_height,
      .format = GPUTextureFormat::RGB32F,
      .data = GPUTextureData::Float,
      .wrap = GPUTextureWrap::Clamp,
      .pixels = pixels.data(),
  });

  msdfgen::destroyFont(font);
  msdfgen::deinitializeFreetype(freetype);
}

auto Font::width(const std::string& s, f32 sx) -> f32 {
  auto text = s.c_str();
  u8 previous = 0;
  auto x = 0.0f;
  for (const char* p = text; *p; p++) {
    auto codepoint = u8(*p);
    if (codepoint >= 128) {
      continue;
    }

    auto& glyph = m_glyphs[codepoint];
    if (previous) {
      x += m_kerning[previous][codepoint] * sx;
    }

    auto x2 = x + glyph.plane_left * sx;
    auto w = (glyph.plane_right - glyph.plane_left) * sx;
    x += glyph.advance * sx;
    previous = codepoint;
  }

  return x;
}

auto Font::render_text(const char* text, f32 x, f32 y, f32 sx, f32 sy) -> void {
  u8 previous = 0;
  for (const char* p = text; *p; p++) {
    auto codepoint = u8(*p);
    if (codepoint >= 128) {
      continue;
    }

    auto& glyph = m_glyphs[codepoint];
    if (previous) {
      x += m_kerning[previous][codepoint] * sx;
    }

    auto x2 = x + glyph.plane_left * sx;
    auto y2 = y - glyph.plane_top * sy;
    auto w = (glyph.plane_right - glyph.plane_left) * sx;
    auto h = (glyph.plane_top - glyph.plane_bottom) * sy;

    x += glyph.advance * sx;
    previous = codepoint;

    if (not glyph.visible or not w or not h or
        m_nvert + 8 >= MAX_VERTICES * 2) {
      continue;
    }

    m_position[m_nvert] = x2;
    m_position[m_nvert + 1] = y2;

    m_position[m_nvert + 2] = x2 + w;
    m_position[m_nvert + 3] = y2;

    m_position[m_nvert + 4] = x2;
    m_position[m_nvert + 5] = y2 + h;

    m_position[m_nvert + 6] = x2 + w;
    m_position[m_nvert + 7] = y2 + h;

    m_uv[m_nvert] = glyph.uv_left;
    m_uv[m_nvert + 1] = glyph.uv_top;

    m_uv[m_nvert + 2] = glyph.uv_right;
    m_uv[m_nvert + 3] = glyph.uv_top;

    m_uv[m_nvert + 4] = glyph.uv_left;
    m_uv[m_nvert + 5] = glyph.uv_bottom;

    m_uv[m_nvert + 6] = glyph.uv_right;
    m_uv[m_nvert + 7] = glyph.uv_bottom;

    m_nvert += 8;
  }
}

auto Font::render_text_center(const char* text, f32 x, f32 y, f32 s) -> void {
  render_text(text, x, y + s * 0.4, s, s);
}

auto Font::submit() -> void {
  if (not m_texture or m_nvert == 0) {
    m_nvert = 0;
    return;
  }

  m_batch->update_binding(0, m_position, sizeof(f32) * m_nvert);
  m_batch->update_binding(1, m_uv, sizeof(f32) * m_nvert);
  m_text_shader->bind();
  m_texture->bind();
  m_text_shader->uniform_int("u_texture", ADDR(0));
  m_text_shader->uniform_float("u_projection", value_ptr(dcx->projection), 16);
  GPUState::get().set_blend(Blend::Alpha);
  dcx->framebuffer->bind();
  m_batch->draw(m_text_shader, (m_nvert >> 3) * 6);
  dcx->framebuffer->unbind();
  GPUState::get().set_blend(Blend::None);
  m_nvert = 0;
}

Font::~Font() {
  delete[] m_position;
  delete[] m_uv;
}

} // namespace zod::sodium
