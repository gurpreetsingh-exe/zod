#include "font.hh"

namespace zod {

constexpr usize MAX_VERTICES = 64 * 1024;

static Unique<Font> g_font = nullptr;

auto init_font(const fs::path& path) -> void {
  g_font = unique<Font>();
  g_font->load_font(path);
}

struct character_info {
  f32 ax; // advance.x
  f32 ay; // advance.y
  f32 bw; // bitmap.width;
  f32 bh; // bitmap.rows;
  f32 bl; // bitmap_left;
  f32 bt; // bitmap_top;
  f32 tx; // x offset of glyph in texture coordinates
} c[128];

auto Font::get() -> Font& { return *g_font; }

Font::Font() {
  if (FT_Init_FreeType(&m_ft)) {
    eprintln("Could not init freetype library");
  }

  m_position = new f32[MAX_VERTICES * 2];
  m_uv = new f32[MAX_VERTICES * 2];
  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, m_position, 2, MAX_VERTICES * 2 },
    { GPUDataType::Float, m_uv, 2, MAX_VERTICES * 2 },
  };
  auto indices = std::vector<u32>();
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
  if (FT_New_Face(m_ft, path.string().c_str(), 0, &m_face)) {
    eprintln("Could not open font");
  }
  FT_Set_Pixel_Sizes(m_face, 0, size);

  FT_GlyphSlot g = m_face->glyph;
  unsigned int w = 0;
  unsigned int h = 0;

  for (int i = 32; i < 128; i++) {
    if (FT_Load_Char(m_face, i, FT_LOAD_RENDER)) {
      eprintln("Loading character {} failed!", i);
      continue;
    }

    w += g->bitmap.width + 1;
    h = std::max(h, g->bitmap.rows);
  }

  m_width = w;
  m_height = h + 2;

  m_texture = GPUBackend::get().create_texture(GPUTextureType::Texture2D,
                                               GPUTextureFormat::Red, m_width,
                                               m_height, /* bindless */ false);
  m_texture->bind();

  int x = 0;
  for (int i = 32; i < 128; i++) {
    if (FT_Load_Char(m_face, i, FT_LOAD_RENDER))
      continue;

    c[i].ax = g->advance.x >> 6;
    c[i].ay = g->advance.y >> 6;
    c[i].bw = g->bitmap.width;
    c[i].bh = g->bitmap.rows + 2;
    c[i].bl = g->bitmap_left;
    c[i].bt = g->bitmap_top;
    c[i].tx = (float)x / m_width;
    m_texture->blit(x + 0.5, 1, g->bitmap.width, g->bitmap.rows,
                    g->bitmap.buffer);
    x += g->bitmap.width + 1;
  }
}

auto Font::render_text(const char* text, f32 x, f32 y, f32 sx, f32 sy) -> void {
  for (const char* p = text; *p; p++) {
    f32 x2 = (x + c[*p].bl * sx);
    f32 y2 = (-y - c[*p].bt * sy);
    f32 w = (c[*p].bw * sx);
    f32 h = (c[*p].bh * sy);

    x += c[*p].ax * sx;
    y += c[*p].ay * sy;

    if (not w or not h) {
      continue;
    }

    m_position[m_nvert] = x2;
    m_position[m_nvert + 1] = -y2;

    m_position[m_nvert + 2] = x2 + w;
    m_position[m_nvert + 3] = -y2;

    m_position[m_nvert + 4] = x2;
    m_position[m_nvert + 5] = -y2 - h;

    m_position[m_nvert + 6] = x2 + w;
    m_position[m_nvert + 7] = -y2 - h;

    m_uv[m_nvert] = c[*p].tx;
    m_uv[m_nvert + 1] = 0;

    m_uv[m_nvert + 2] = c[*p].tx + c[*p].bw / m_width;
    m_uv[m_nvert + 3] = 0;

    m_uv[m_nvert + 4] = c[*p].tx;
    m_uv[m_nvert + 5] = c[*p].bh / m_height;

    m_uv[m_nvert + 6] = c[*p].tx + c[*p].bw / m_width;
    m_uv[m_nvert + 7] = c[*p].bh / m_height;

    m_nvert += 8;
  }
}

auto Font::render_text_center(const char* text, f32 x, f32 y, f32 s) -> void {
  render_text(text, x, y - size * s * 0.25, s, s);
}

auto Font::submit() -> void {
  m_batch->update_binding(0, m_position, sizeof(f32) * m_nvert);
  m_batch->update_binding(1, m_uv, sizeof(f32) * m_nvert);
  m_text_shader->bind();
  m_texture->bind();
  m_text_shader->uniform_int("u_texture", ADDR(0));
  m_batch->draw(m_text_shader, (m_nvert >> 3) * 6);
  m_nvert = 0;
}

Font::~Font() {
  delete[] m_position;
  delete[] m_uv;
  FT_Done_FreeType(m_ft);
}

} // namespace zod
