#include "sodium/gui.hh"
#include "gpu/backend.hh"
#include "gpu/texture.hh"
#include "sodium/theme.hh"

#include "shaders_generated.hh"

#include <msdfgen-ext.h>
#include <msdfgen.h>

namespace zod {

namespace sodium {

struct WidgetInfo {
  rect _rect;
};

struct DrawData {
  SharedPtr<GPUBuffer> draw_data;
  SharedPtr<GPUBuffer> colors;
  SharedPtr<GPUTexture> white_texture;
  SharedPtr<GPUShader> shader;
  SharedPtr<GPUShader> present;
  SharedPtr<GPUBatch> batch;
  SharedPtr<GPUFrameBuffer> framebuffer;
  Vector<WidgetInfo> widget_info = {};
};

DrawData* dcx = nullptr;
mat4 projection = mat4(1.0f);

///////////////////////////////////////////////////////////
/// Font //////////////////////////////////////////////////
///////////////////////////////////////////////////////////

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
  m_text_shader->uniform_float("u_projection", value_ptr(projection), 16);
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

///////////////////////////////////////////////////////////
/// Images/Icons //////////////////////////////////////////
///////////////////////////////////////////////////////////

struct TexturedRect {
  Rect rect;
  SharedPtr<GPUTexture> image = nullptr;
};

static Vector<TexturedRect> g_textured_rects = {};

constexpr auto IconAtlasColumns = 8.0f;
constexpr auto IconAtlasRows = 8.0f;

struct IconAtlas {
  SharedPtr<GPUTexture> texture = nullptr;
  PaintCx cx = {};

  IconAtlas() {
    texture = GPUBackend::get().create_texture({
        .name = "sodium.custom.blender_icons",
        .wrap = GPUTextureWrap::Clamp,
        .path = "icons.png",
    });
  }

  auto uv(IconId id) const -> Rect {
    auto idx = int(id);
    if (idx < 0) {
      return {};
    }
    auto w = 1.0f / IconAtlasColumns;
    auto h = 1.0f / IconAtlasRows;
    return { { f32(idx) * w, 1.0f - h }, { w, h } };
  }
};

auto icon_atlas() -> IconAtlas& {
  static auto atlas = IconAtlas();
  return atlas;
}

auto icon(IconId id, Rect bounds, vec4 color) -> void {
  if (id == IconId::None) {
    return;
  }
  auto& atlas = icon_atlas();

  auto textured_rect = [](PaintCx& cx, Rect rect, Rect uv, vec4 color) -> void {
    auto start = cx.draw_list.draw_data.size();
    cx.draw_list.draw_data.push_back(
        { rect.position + vec2 { 0.0f, rect.size.y },
          uv.position + vec2 { 0.0f, uv.size.y } });
    cx.draw_list.draw_data.push_back(
        { rect.position + rect.size, uv.position + uv.size });
    cx.draw_list.draw_data.push_back(
        { rect.position + vec2 { rect.size.x, 0.0f },
          uv.position + vec2 { uv.size.x, 0.0f } });
    cx.draw_list.draw_data.push_back({ rect.position, uv.position });
    cx.draw_list.indices.push_back(start);
    cx.draw_list.indices.push_back(start + 1);
    cx.draw_list.indices.push_back(start + 2);
    cx.draw_list.indices.push_back(start + 2);
    cx.draw_list.indices.push_back(start + 3);
    cx.draw_list.indices.push_back(start);
    cx.draw_list.colors.push_back(color);
    cx.draw_list.colors.push_back(color);
  };

  textured_rect(atlas.cx, bounds, atlas.uv(id), color);
}

auto flush_icons() -> void {
  auto& atlas = icon_atlas();
  if (not atlas.cx.draw_list.indices.empty()) {
    submit(atlas.cx, atlas.texture);
  }
}

auto resize(vec2 size) -> void {
  projection = ortho(0.f, size.x, size.y, 0.f, -1.f, 1.f);
  dcx->framebuffer->resize(size.x, size.y);
}

auto make_id() -> usize {
  static usize ID = 0;
  return ID++;
}

auto rect_(PaintCx& pcx, vec4 col, vec2 a, vec2 b) -> void {
  auto start = pcx.draw_list.draw_data.size();

  auto a0 = vec2(a.x, a.y + b.y);
  auto a1 = a + b;
  auto a2 = vec2(a.x + b.x, a.y);
  auto a3 = a;

  pcx.draw_list.draw_data.push_back({ a0, { 0, 0 } });
  pcx.draw_list.draw_data.push_back({ a1, { 1, 0 } });
  pcx.draw_list.draw_data.push_back({ a2, { 1, 1 } });
  pcx.draw_list.draw_data.push_back({ a3, { 0, 1 } });

  pcx.draw_list.indices.push_back(start);
  pcx.draw_list.indices.push_back(start + 1);
  pcx.draw_list.indices.push_back(start + 2);
  pcx.draw_list.indices.push_back(start + 2);
  pcx.draw_list.indices.push_back(start + 3);
  pcx.draw_list.indices.push_back(start);

  pcx.draw_list.colors.push_back(col);
  pcx.draw_list.colors.push_back(col);
}

///////////////////////////////////////////////////////////
/// Rect //////////////////////////////////////////////////
///////////////////////////////////////////////////////////
auto rect::paint(PaintCx& pcx) const -> void {
  const auto position = inner.position;
  const auto size = inner.size;
  rect_(pcx, color, position + padding, size - padding * 2);
}

auto rect::intersect(vec2 point) const -> bool {
  const auto position = inner.position;
  const auto size = inner.size;
  return Rect { position + padding, size - padding }.intersect(point);
}

auto rect::operator->() -> Rect* { return &inner; }
auto rect::operator*() -> Rect& { return inner; }

///////////////////////////////////////////////////////////
auto init_gui() -> void {
  Theme::load();

  auto& backend = GPUBackend::get();
  auto draw_data = backend.create_buffer(
      { "gui::draw_data", GPUBufferUsage::Storage, 640000 });
  auto colors =
      backend.create_buffer({ "gui::colors", GPUBufferUsage::Storage, 640000 });
  auto white_pixel = u32(0xffffffff);
  auto white_texture = backend.create_texture({
      .name = "gui::white_texture",
      .width = 1,
      .height = 1,
      .format = GPUTextureFormat::RGBA8,
      .wrap = GPUTextureWrap::Clamp,
      .pixels = &white_pixel,
  });

  auto batch = backend.create_batch({}, Vector<u32>(10000));
  auto shader = backend.create_shader(GPUShaderCreateInfo("gui::draw")
                                          .vertex_source(g_view2d_vert_src)
                                          .fragment_source(g_draw_src));
  auto present = backend.create_shader(GPUShaderCreateInfo("gui::present")
                                           .vertex_source(g_fullscreen_src)
                                           .fragment_source(g_texture_src));
  auto framebuffer = backend.create_framebuffer({
      .name = "sodium::framebuffer",
      .width = 4,
      .height = 4,
  });
  framebuffer->add_color_attachment(backend.create_texture(
      { .name = "sodium::framebuffer::color", .width = 4, .height = 4 }));
  dcx = new DrawData(draw_data, colors, white_texture, shader, present, batch,
                     framebuffer);
}

auto submit(PaintCx& cx, SharedPtr<GPUTexture> texture) -> void {
  dcx->draw_data->write(cx.draw_list.draw_data.data(),
                        cx.draw_list.draw_data.size() * sizeof(GPUDrawData));
  dcx->colors->write(cx.draw_list.colors.data(),
                     cx.draw_list.colors.size() * sizeof(vec4));
  dcx->batch->get_index_buffer()->write(
      cx.draw_list.indices.data(), cx.draw_list.indices.size() * sizeof(u32));

  dcx->draw_data->bind(1);
  dcx->colors->bind(2);

  dcx->shader->bind();
  texture->bind();
  dcx->shader->uniform_int("u_texture", ADDR(0));
  dcx->shader->uniform_float("u_projection", value_ptr(projection), 16);
  dcx->framebuffer->bind();
  dcx->batch->draw(dcx->shader, cx.draw_list.indices.size());
  dcx->framebuffer->unbind();

  cx.draw_list.draw_data.clear();
  cx.draw_list.colors.clear();
  cx.draw_list.indices.clear();
}

auto present() -> void {
  dcx->present->bind();
  dcx->framebuffer->get_slot(0)->bind();
  dcx->present->uniform_int("u_texture", ADDR(0));
  GPUState::get().draw_immediate(3);
}

auto submit(PaintCx& cx) -> void { submit(cx, dcx->white_texture); }

auto draw_images() -> void {
  auto cx = PaintCx();
  for (auto& r : g_textured_rects) {
    rect(r.rect, vec4(1.0f)).paint(cx);
    submit(cx, r.image);
  }
  g_textured_rects.clear();
}

auto operator~(Axis axis) -> Axis {
  if (axis == Axis::Horizontal) {
    return Axis::Vertical;
  }
  return Axis::Horizontal;
}

auto axis_padding_start(const Padding& padding, Axis axis) -> f32 {
  if (axis == Axis::Horizontal) {
    return padding.left;
  }
  return padding.top;
}

auto axis_size_rule(const SlotStyle& style, Axis axis) -> SizeRule {
  if (axis == Axis::Horizontal) {
    return style.horizontal_rule;
  }
  return style.vertical_rule;
}

auto axis_alignment(const SlotStyle& style, Axis axis) -> Align {
  if (axis == Axis::Horizontal) {
    return style.horizontal_alignment;
  }
  return style.vertical_alignment;
}

auto auto_or_fixed_size(SizeRule rule, f32 fixed, f32 desired) -> f32 {
  if (rule == SizeRule::Fixed) {
    return fixed;
  }
  if (rule == SizeRule::Auto) {
    return std::max(0.0f, desired);
  }
  return 0.0f;
}

auto axis_auto_or_fixed_size(const SlotStyle& style, const vec2& desired,
                             Axis axis) -> f32 {
  return auto_or_fixed_size(axis_size_rule(style, axis), style.size[axis],
                            desired[axis]);
}

auto slot_size(const SlotStyle& style, const vec2& desired) -> vec2 {
  return { axis_auto_or_fixed_size(style, desired, Axis::Horizontal),
           axis_auto_or_fixed_size(style, desired, Axis::Vertical) };
}

auto overlay_axis_size(const SlotStyle& style, const vec2& desired,
                       f32 available, bool allow_stretch, Axis axis) -> f32 {
  auto rule = axis_size_rule(style, axis);
  if (rule == SizeRule::Fixed) {
    return style.size[axis];
  }
  if (allow_stretch and rule == SizeRule::Stretch) {
    return available;
  }
  return std::max(0.0f, desired[axis]);
}

auto overlay_width(const SlotStyle& style, const vec2& desired, f32 available,
                   bool allow_stretch) -> f32 {
  return overlay_axis_size(style, desired, available, allow_stretch,
                           Axis::Horizontal);
}

auto overlay_height(const SlotStyle& style, const vec2& desired, f32 available,
                    bool allow_stretch) -> f32 {
  return overlay_axis_size(style, desired, available, allow_stretch,
                           Axis::Vertical);
}

auto push_filled_rect(PaintCx& cx, const Rect& frame, const vec4& color)
    -> void {
  if (color[3] > 0.0f) {
    rect(frame, color).paint(cx);
  }
}

auto align_axis(Rect& result, const Rect& outer, const vec2& desired, Axis axis,
                Align alignment) -> void {
  if (alignment == Align::Stretch) {
    result.size[axis] = outer.size[axis];
    return;
  }

  auto size = std::min(desired[axis], outer.size[axis]);
  result.size[axis] = size;

  auto free_space = outer.size[axis] - size;
  if (alignment == Align::Center) {
    result.position[axis] += free_space * 0.5f;
  } else if (alignment == Align::End) {
    result.position[axis] += free_space;
  }
}

auto align_within(const Rect& outer, const vec2& desired, Align horizontal,
                  Align vertical) -> Rect {
  auto result = outer;
  auto desired_size = glm::max(vec2 { 0.0f }, desired);

  align_axis(result, outer, desired_size, Axis::Horizontal, horizontal);
  align_axis(result, outer, desired_size, Axis::Vertical, vertical);

  return result;
}

auto measure_stack_children(const Vector<Container::Slot>& children,
                            const WidgetStyle& style, Axis axis, vec2 available)
    -> vec2 {
  auto opposite_axis = ~axis;
  auto used = vec2();
  auto visible = usize(0);

  for (const auto& slot : children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }

    auto slot_box = Rect { 0.0f, 0.0f, available.x, available.y };
    auto inner = slot_box.padding(slot.style.padding);
    auto desired = slot.child->compute_desired_size(inner.size);

    auto child_size =
        slot_size(slot.style, desired) + slot.style.padding.combined();

    used[axis] += child_size[axis];
    used[opposite_axis] =
        std::max(used[opposite_axis], child_size[opposite_axis]);
    ++visible;
  }

  auto gaps = 0.0f;
  if (visible > 1) {
    gaps = style.gap * f32(visible - 1);
  }

  used[axis] += gaps;
  return used + style.padding.combined();
}

static auto event_reply(EventResponse widget_reply, EventResponse child_reply,
                        bool handled) -> EventResponse {
  if (widget_reply) {
    return widget_reply;
  }
  if (child_reply) {
    return child_reply;
  }
  if (handled) {
    return EventResponse::handled();
  }
  return EventResponse::unhandled();
}

auto Widget::event(const Event& event) -> EventResponse {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    m_hovered = false;
    return EventResponse::unhandled();
  }

  auto child_reply = EventResponse::unhandled();
  for (auto child : get_children()) {
    if (child) {
      auto reply = child->event(event);
      if (reply) {
        child_reply = reply;
      }
    }
  }

  // TODO: key events on hover??
  m_hovered = m_visibility != Visibility::SelfHitTestInvisible and
              m_frame.intersect(event.mouse);
  if (not m_hovered) {
    return EventResponse::handled();
  }

  switch (event.kind) {
    case Event::MouseDown:
      return event_reply(apply_event_reply(event, on_mouse_down(event)),
                         child_reply, m_hovered);
    case Event::MouseUp:
      return event_reply(apply_event_reply(event, on_mouse_up(event)),
                         child_reply, m_hovered);
    case Event::MouseMove: {
      auto widget_reply = drag_detected_reply(event);
      if (not widget_reply) {
        widget_reply = on_mouse_move(event);
      }
      return event_reply(widget_reply, child_reply, m_hovered);
    }
    case Event::KeyDown:
      return event_reply(on_key_down(event), child_reply, false);
    case Event::KeyUp:
      return event_reply(on_key_up(event), child_reply, false);
    case Event::KeyRepeat:
      return event_reply(on_key_repeat(event), child_reply, false);
    case Event::WindowResize:
      return event_reply(on_window_resize(event), child_reply, false);
    case Event::WindowClose:
      return event_reply(on_window_close(event), child_reply, false);
    case Event::None:
      break;
  }

  return child_reply;
}

auto Widget::apply_event_reply(const Event& event, EventResponse reply)
    -> EventResponse {
  if (event.kind == Event::MouseDown and reply.wants_drag_detection and
      event.button == reply.drag_button and m_hovered) {
    m_detecting_drag = true;
    m_drag_detected = false;
    m_drag_button = reply.drag_button;
    m_drag_start = event.mouse;
  }

  if (event.kind == Event::MouseUp and event.button == m_drag_button) {
    m_detecting_drag = false;
    m_drag_detected = false;
    m_drag_button = MouseButton::None;
  }

  return reply;
}

auto Widget::drag_detected_reply(const Event& event) -> EventResponse {
  if (event.kind != Event::MouseMove or not m_detecting_drag or
      m_drag_detected) {
    return EventResponse::unhandled();
  }

  auto delta = event.mouse - m_drag_start;
  auto moved = delta.x * delta.x + delta.y * delta.y >
               m_drag_threshold * m_drag_threshold;
  if (not moved) {
    return EventResponse::unhandled();
  }

  m_drag_detected = true;
  return on_drag_detected(event);
}

auto Widget::on_mouse_down(const Event& event) -> EventResponse {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->mouse_down) {
      return metadata->mouse_down.execute(event);
    }
  }
  return EventResponse::unhandled();
}

auto Widget::on_mouse_up(const Event& event) -> EventResponse {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->mouse_up) {
      return metadata->mouse_up.execute(event);
    }
  }
  return EventResponse::unhandled();
}

auto Widget::on_mouse_move(const Event& event) -> EventResponse {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->mouse_move) {
      return metadata->mouse_move.execute(event);
    }
  }
  return EventResponse::unhandled();
}

auto Widget::on_drag_detected(const Event& event) -> EventResponse {
  if (auto metadata = get_metadata<WidgetMouseEventsMetaData>()) {
    if (metadata->drag_detected) {
      return metadata->drag_detected.execute(event);
    }
  }
  return EventResponse::unhandled();
}

auto Widget::push_self_draws(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }

  if (m_style.border_thickness > 0.0f and m_style.border_color[3] > 0.0f) {
    auto t = m_style.border_thickness;
    auto top_left = m_frame.position;
    auto bottom_right = m_frame.position + m_frame.size - vec2 { t, t };
    push_filled_rect(cx, { top_left, { m_frame.size.x, t } },
                     m_style.border_color);
    push_filled_rect(cx,
                     { { top_left.x, bottom_right.y }, { m_frame.size.x, t } },
                     m_style.border_color);
    push_filled_rect(cx, { top_left, { t, m_frame.size.y } },
                     m_style.border_color);
    push_filled_rect(cx,
                     { { bottom_right.x, top_left.y }, { t, m_frame.size.y } },
                     m_style.border_color);
  }

  if (m_style.background[3] > 0.0f) {
    auto t = std::max(0.0f, m_style.border_thickness);
    auto inner = m_frame.padding(m_style.padding).padding(t);
    push_filled_rect(cx, inner, m_style.background);
  }
}

auto CompoundWidget::set_child(SharedPtr<Widget> child, const SlotStyle& style)
    -> SingleChildSlot& {
  m_child_slot = { child, style };
  return m_child_slot;
}

auto CompoundWidget::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  if (not m_child_slot.child) {
    return cache_desired_size(m_style.padding.combined());
  }

  auto inner = Rect { {}, available }.padding(m_style.padding);
  auto child_inner = inner.padding(m_child_slot.style.padding);
  auto desired = m_child_slot.child->compute_desired_size(child_inner.size);

  return cache_desired_size(desired + m_style.padding.combined() +
                            m_child_slot.style.padding.combined());
}

auto CompoundWidget::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  if (not m_child_slot.child) {
    return;
  }

  auto child_outer =
      m_frame.padding(m_style.padding).padding(m_child_slot.style.padding);
  auto desired = m_child_slot.child->desired_size();
  auto aligned = align_within(child_outer, desired,
                              m_child_slot.style.horizontal_alignment,
                              m_child_slot.style.vertical_alignment);
  m_child_slot.child->arrange(aligned);
}

auto CompoundWidget::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }
  push_self_draws(cx);
  if (m_child_slot.child) {
    m_child_slot.child->paint(cx);
  }
}

auto Box::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  auto child_desired = CompoundWidget::compute_desired_size(available);
  auto desired = child_desired;
  if (m_desired_size.x > 0.0f) {
    desired.x = m_desired_size.x;
  }
  if (m_desired_size.y > 0.0f) {
    desired.y = m_desired_size.y;
  }
  return cache_desired_size(desired);
}

auto Image::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  return cache_desired_size(m_desired_size);
}

auto Image::arrange(const Rect& bounds) -> void { m_frame = bounds; }

auto Image::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }

  g_textured_rects.emplace_back(m_frame, m_image);

  // push_self_draws(cx);
}

constexpr f32 FontSizeInMenu = 12;

auto Button::on_mouse_down(const Event& event) -> EventResponse {
  if (not m_enabled) {
    m_hovered = false;
    m_pressed = false;
    return EventResponse::unhandled();
  }

  if (event.button == MouseButton::Left and m_hovered) {
    m_pressed = true;
    return EventResponse::handled();
  }

  return EventResponse::unhandled();
}

auto Button::on_mouse_up(const Event& event) -> EventResponse {
  if (not m_enabled) {
    m_hovered = false;
    m_pressed = false;
    return EventResponse::unhandled();
  }

  if (event.button == MouseButton::Left) {
    auto was_pressed = m_pressed;
    m_pressed = false;
    if (was_pressed) {
      if (m_hovered and m_on_clicked) {
        m_on_clicked.execute(event);
      }
      return EventResponse::handled();
    }
  }

  return EventResponse::unhandled();
}

auto Button::on_mouse_move(const Event&) -> EventResponse {
  if (not m_enabled) {
    m_hovered = false;
    m_pressed = false;
    return EventResponse::unhandled();
  }

  if (m_hovered) {
    return EventResponse::handled();
  }

  return EventResponse::unhandled();
}

auto Button::compute_desired_size(vec2 available) -> vec2 {
  auto width = Font::get().width(m_name, FontSizeInMenu);
  auto desired =
      max(vec2(width, FontSizeInMenu) + m_style.padding.combined(), m_min_size);
  set_desired_size(desired);
  return desired;
  // return cache_desired_size(
  //     CompoundWidget::compute_desired_size({ width, available.y }));
}

auto Button::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }
  push_filled_rect(cx, m_frame,
                   m_hovered ? m_button_style.hovered_tint
                             : m_style.background);

  auto center = m_frame.position + m_frame.size * 0.5f;
  if (m_icon != IconId::None) {
    auto size = vec2(0.6f) * std::min(m_frame.size.x, m_frame.size.y);
    icon(m_icon, { center - size * 0.5f, size }, { 1, 1, 1, 1 });
  } else {
    auto w = Font::get().width(m_name, FontSizeInMenu);
    Font::get().render_text_center(m_name.c_str(), center.x - w * 0.5, center.y,
                                   FontSizeInMenu);
  }
}

auto Menu::compute_desired_size(vec2 available) -> vec2 {
  auto width = Font::get().width(m_name, FontSizeInMenu);
  auto desired =
      max(vec2(width, FontSizeInMenu) + m_style.padding.combined(), m_min_size);
  set_desired_size(desired);
  if (m_open) {
    auto desired_size = vec2();
    for (auto button : m_buttons) {
      desired_size = max(button->compute_desired_size(available), desired_size);
    }
  }

  return desired;
}

auto Menu::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }
  push_filled_rect(cx, m_frame, m_style.background);
  auto w = Font::get().width(m_name, FontSizeInMenu);
  auto center = m_frame.position + m_frame.size * 0.5f;
  Font::get().render_text_center(m_name.c_str(), center.x - w * 0.5, center.y,
                                 FontSizeInMenu);
}

auto Menu::arrange(const Rect& bounds) -> void { m_frame = bounds; }

auto Menu::get_children() const -> WidgetChildren {
  auto children = WidgetChildren {};
  children.reserve(m_buttons.size());
  for (const auto& button : m_buttons) {
    if (button) {
      children.push_back(button);
    }
  }
  return children;
}

auto Container::add_child(SharedPtr<Widget> child, const SlotStyle& style)
    -> Container::Slot& {
  m_children.push_back({ child, style });
  return m_children.back();
}

auto Container::get_children() const -> WidgetChildren {
  auto children = WidgetChildren {};
  children.reserve(m_children.size());
  for (const auto& slot : m_children) { children.push_back(slot.child); }
  return children;
}

auto BoxContainer::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  return cache_desired_size(
      measure_stack_children(m_children, m_style, m_axis, available));
}

auto BoxContainer::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  auto inner = m_frame.padding(m_style.padding);
  auto layout_axis = m_axis;
  auto opposite_axis = ~layout_axis;

  auto fixed_layout_size = 0.0f;
  auto stretch_weight_sum = 0.0f;
  auto visible = usize(0);

  for (const auto& slot : m_children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }
    ++visible;
    auto layout_rule = axis_size_rule(slot.style, layout_axis);
    auto layout_padding = slot.style.padding.combined()[layout_axis];

    fixed_layout_size += layout_padding;
    if (layout_rule == SizeRule::Stretch) {
      stretch_weight_sum += std::max(0.0f, slot.style.stretch_weight);
    } else if (layout_rule == SizeRule::Fixed) {
      fixed_layout_size += slot.style.size[layout_axis];
    } else {
      auto desired = slot.child->desired_size();
      auto desired_layout_size = desired[layout_axis];
      if (desired_layout_size > 0.0f) {
        fixed_layout_size += desired_layout_size;
      }
    }
  }

  auto gaps = 0.0f;
  if (visible > 1) {
    gaps = m_style.gap * f32(visible - 1);
  }

  auto available_layout_size = inner.size[layout_axis];
  auto available_opposite_size = inner.size[opposite_axis];
  auto cursor = inner.position[layout_axis];

  auto remaining_layout_size =
      std::max(0.0f, available_layout_size - fixed_layout_size - gaps);

  for (auto& slot : m_children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }
    auto child_desired = slot.child->desired_size();
    auto layout_rule = axis_size_rule(slot.style, layout_axis);
    auto opposite_rule = axis_size_rule(slot.style, opposite_axis);
    auto fixed_layout_size = slot.style.size[layout_axis];
    auto fixed_opposite_size = slot.style.size[opposite_axis];
    auto desired_layout_size = child_desired[layout_axis];
    auto desired_opposite_size = child_desired[opposite_axis];
    auto opposite_alignment = axis_alignment(slot.style, opposite_axis);
    auto layout_padding = slot.style.padding.combined()[layout_axis];
    auto opposite_padding = slot.style.padding.combined()[opposite_axis];
    auto slot_opposite_size =
        std::max(0.0f, available_opposite_size - opposite_padding);

    auto layout_size = 0.0f;
    if (layout_rule == SizeRule::Fixed) {
      layout_size = fixed_layout_size;
    } else if (layout_rule == SizeRule::Stretch) {
      auto weight = 1.0f / std::max(1ul, visible);
      if (stretch_weight_sum > 0.0f) {
        weight = std::max(0.0f, slot.style.stretch_weight) / stretch_weight_sum;
      }
      layout_size = remaining_layout_size * weight;
    } else {
      layout_size = std::max(0.0f, desired_layout_size);
    }

    auto opposite_size = 0.0f;
    if (opposite_rule == SizeRule::Fixed) {
      opposite_size = fixed_opposite_size;
    } else if (opposite_rule == SizeRule::Stretch) {
      opposite_size = slot_opposite_size;
    } else if (opposite_alignment == Align::Stretch) {
      opposite_size = slot_opposite_size;
    } else {
      opposite_size = std::max(0.0f, desired_opposite_size);
    }

    auto child_outer = Rect {};
    child_outer.position[layout_axis] =
        cursor + axis_padding_start(slot.style.padding, layout_axis);
    child_outer.position[opposite_axis] =
        inner.position[opposite_axis] +
        axis_padding_start(slot.style.padding, opposite_axis);
    child_outer.size[layout_axis] = layout_size;
    child_outer.size[opposite_axis] = opposite_size;

    auto aligned = align_within(child_outer, child_desired,
                                slot.style.horizontal_alignment,
                                slot.style.vertical_alignment);
    slot.child->arrange(aligned);
    cursor += layout_size + layout_padding + m_style.gap;
  }
}

auto BoxContainer::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }
  push_self_draws(cx);
  for (const auto& slot : m_children) {
    if (slot.child) {
      slot.child->paint(cx);
    }
  }
}

auto DockArea::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  auto padding = m_style.padding.combined();
  auto horizontal_size = padding.x;
  auto vertical_size = padding.y;
  auto center_w = 0.0f;
  auto center_h = 0.0f;

  for (usize i = 0; i < m_children.size(); ++i) {
    const auto& slot = m_children[i];
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }
    if (slot.style.dock == Dock::Tab and m_active_tab and *m_active_tab != i) {
      continue;
    }

    auto desired = slot.child->compute_desired_size(available);

    switch (slot.style.dock) {
      case Dock::Left:
      case Dock::Right:
        horizontal_size +=
            axis_auto_or_fixed_size(slot.style, desired, Axis::Horizontal) +
            slot.style.padding.horizontal();
        center_h = std::max(center_h, std::max(0.0f, desired.y) +
                                          slot.style.padding.vertical());
        break;
      case Dock::Top:
      case Dock::Bottom:
        vertical_size +=
            axis_auto_or_fixed_size(slot.style, desired, Axis::Vertical) +
            slot.style.padding.vertical();
        center_w = std::max(center_w, std::max(0.0f, desired.x) +
                                          slot.style.padding.horizontal());
        break;
      case Dock::Fill:
      case Dock::Tab:
        break;
    }
  }

  return cache_desired_size(
      { horizontal_size + center_w, vertical_size + center_h });
}

auto DockArea::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  auto remaining = m_frame.padding(m_style.padding);
  auto active = m_active_tab.value_or(usize(-1));

  for (usize i = 0; i < m_children.size(); ++i) {
    auto& slot = m_children[i];
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }
    if (slot.style.dock == Dock::Tab and i != active) {
      continue;
    }

    auto desired = slot.child->desired_size();
    auto child = remaining.padding(slot.style.padding);

    switch (slot.style.dock) {
      case Dock::Top: {
        auto h = axis_auto_or_fixed_size(slot.style, desired, Axis::Vertical);
        child.size.y = h;
        slot.child->arrange(child);
        auto consumed = h + slot.style.padding.vertical();
        remaining.position.y += consumed;
        remaining.size.y = std::max(0.0f, remaining.size.y - consumed);
        break;
      }
      case Dock::Bottom: {
        auto h = axis_auto_or_fixed_size(slot.style, desired, Axis::Vertical);
        child.position.y = remaining.position.y + remaining.size.y - h;
        child.size.y = h;
        slot.child->arrange(child);
        auto consumed = h + slot.style.padding.vertical();
        remaining.size.y = std::max(0.0f, remaining.size.y - consumed);
        break;
      }
      case Dock::Left: {
        auto w = axis_auto_or_fixed_size(slot.style, desired, Axis::Horizontal);
        child.size.x = w;
        slot.child->arrange(child);
        auto consumed = w + slot.style.padding.horizontal();
        remaining.position.x += consumed;
        remaining.size.x = std::max(0.0f, remaining.size.x - consumed);
        break;
      }
      case Dock::Right: {
        auto w = axis_auto_or_fixed_size(slot.style, desired, Axis::Horizontal);
        child.position.x = remaining.position.x + remaining.size.x - w;
        child.size.x = w;
        slot.child->arrange(child);
        auto consumed = w + slot.style.padding.horizontal();
        remaining.size.x = std::max(0.0f, remaining.size.x - consumed);
        break;
      }
      case Dock::Fill:
      case Dock::Tab:
        slot.child->arrange(child);
        break;
    }
  }
}

auto DockArea::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }
  push_self_draws(cx);
  for (usize i = 0; i < m_children.size(); ++i) {
    const auto& slot = m_children[i];
    if (not slot.child) {
      continue;
    }
    if (slot.style.dock == Dock::Tab and m_active_tab and *m_active_tab != i) {
      continue;
    }
    slot.child->paint(cx);
  }
}

auto Border::paint(PaintCx& cx) const -> void { CompoundWidget::paint(cx); }

auto DPIScaler::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }
  auto safe_scale = std::max(0.0001f, m_scale);
  auto unscaled = vec2 { available.x / safe_scale, available.y / safe_scale };
  auto desired = CompoundWidget::compute_desired_size(unscaled);
  return cache_desired_size(desired * safe_scale);
}

auto DPIScaler::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  if (not m_child_slot.child) {
    return;
  }

  auto safe_scale = std::max(0.0001f, m_scale);
  auto scaled_bounds = Rect { bounds.position, bounds.size / safe_scale };
  CompoundWidget::arrange(scaled_bounds);
  m_frame = bounds;
}

auto Overlay::add_overlay_child(SharedPtr<Widget> child, const SlotStyle& style,
                                int layer) -> Overlay::OverlaySlot& {
  m_overlay_children.push_back({ child, style, layer });
  return m_overlay_children.back();
}

auto Overlay::compute_desired_size(vec2 available) -> vec2 {
  if (m_visibility == Visibility::Collapsed) {
    return cache_desired_size({});
  }

  auto padding = m_style.padding.combined();
  auto horizontal_size = padding.x;
  auto vertical_size = padding.y;
  auto inner = Rect { {}, available }.padding(m_style.padding);

  for (const auto& slot : m_overlay_children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }

    auto slot_inner = inner.padding(slot.style.padding);
    auto desired = slot.child->compute_desired_size(slot_inner.size);
    auto slot_w = overlay_width(slot.style, desired, slot_inner.size.x, false);
    auto slot_h = overlay_height(slot.style, desired, slot_inner.size.y, false);
    auto slot_padding = slot.style.padding.combined();
    horizontal_size =
        std::max(horizontal_size, slot_w + padding.x + slot_padding.x);
    vertical_size =
        std::max(vertical_size, slot_h + padding.y + slot_padding.y);
  }

  return cache_desired_size({ horizontal_size, vertical_size });
}

auto Overlay::arrange(const Rect& bounds) -> void {
  m_frame = bounds;
  auto inner = m_frame.padding(m_style.padding);

  for (auto& slot : m_overlay_children) {
    if (not slot.child or slot.child->visibility() == Visibility::Collapsed) {
      continue;
    }

    auto slot_inner = inner.padding(slot.style.padding);
    auto desired = slot.child->desired_size();
    auto horizontal_size =
        overlay_width(slot.style, desired, slot_inner.size.x, true);
    auto vertical_size =
        overlay_height(slot.style, desired, slot_inner.size.y, true);
    auto aligned = align_within(slot_inner, { horizontal_size, vertical_size },
                                slot.style.horizontal_alignment,
                                slot.style.vertical_alignment);
    slot.child->arrange(aligned);
  }
}

auto Overlay::paint(PaintCx& cx) const -> void {
  if (m_visibility == Visibility::Hidden or
      m_visibility == Visibility::Collapsed) {
    return;
  }

  push_self_draws(cx);
  auto slots = Vector<const OverlaySlot*> {};
  slots.reserve(m_overlay_children.size());
  for (const auto& slot : m_overlay_children) { slots.push_back(&slot); }
  std::stable_sort(slots.begin(), slots.end(),
                   [](const OverlaySlot* a, const OverlaySlot* b) {
                     return a->layer < b->layer;
                   });

  for (const auto* slot : slots) {
    if (slot->child) {
      slot->child->paint(cx);
    }
  }
}

auto Overlay::get_children() const -> WidgetChildren {
  auto children = Container::get_children();
  children.reserve(children.size() + m_overlay_children.size());
  for (const auto& slot : m_overlay_children) {
    children.push_back(slot.child);
  }
  return children;
}

// auto title_button(String name, vec4 image_color, vec4 button_color)
//     -> SharedPtr<Widget> {
//   return (create<Button>()
//               .name(name)
//               .is_focusable(false)
//               .is_enabled(true)
//               .content_padding(8.0f)
//               .button_style(button_color)[create<DPIScaler>(
//                   1.0f)[create<Image>()
//                             .name(name + "_icon")
//                             .image(image_color)
//                             .desired_size({ 12.0f, 12.0f })
//                             .accessible_text(std::move(name))]])
//       .build();
// }

TitleBar::TitleBar() {
  set_name("title_bar");
  m_style.background = { 0.06f, 0.07f, 0.10f, 1.0f };

  // auto controls = (create<HorizontalBox>()
  //                      .name("title_bar_controls")
  //                      .visibility(Visibility::SelfHitTestInvisible) +
  //                  slot().auto_width().stretch_height()[title_button(
  //                      "minimize", { 0.72f, 0.76f, 0.84f, 1.0f },
  //                      { 0.10f, 0.11f, 0.15f, 1.0f })] +
  //                  slot().auto_width().stretch_height()[title_button(
  //                      "maximize", { 0.72f, 0.76f, 0.84f, 1.0f },
  //                      { 0.10f, 0.11f, 0.15f, 1.0f })] +
  //                  slot().auto_width().stretch_height()[title_button(
  //                      "close", { 0.95f, 0.35f, 0.28f, 1.0f },
  //                      { 0.16f, 0.08f, 0.08f, 1.0f })])
  //                     .build();
  //
  // auto control_box =
  //     (create<Box>()
  //          .name("title_bar_control_box")
  //          .visibility(Visibility::SelfHitTestInvisible)[controls])
  //         .build();
  //
  // auto row =
  //     (create<HorizontalBox>()
  //          .name("title_bar_row")
  //          .visibility(Visibility::SelfHitTestInvisible) +
  //      slot()
  //          .stretch_width()
  //          .stretch_height()[create<Box>()
  //                                .name("title_bar_drag_region")
  //                                .background({ 0.06f, 0.07f, 0.10f, 1.0f })]
  //                                +
  //      slot().auto_width().stretch_height().halign(Align::End)[control_box])
  //         .build();
  //
  // set_child((create<Overlay>().name("title_bar_overlay") +
  //            slot().layer(0).stretch_width().stretch_height()[row] +
  //            slot().layer(1).fixed_width(2.0f).stretch_height().halign(
  //                Align::End)[create<Box>()
  //                                .name("title_bar_right_edge")
  //                                .background({ 0.22f, 0.24f, 0.30f, 1.0f })])
  //               .build());
}

} // namespace sodium

} // namespace zod
