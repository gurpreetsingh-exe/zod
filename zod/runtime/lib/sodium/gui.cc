#include "sodium/gui.hh"
#include "gpu/backend.hh"
#include "sodium/font.hh"
#include "sodium/icon.hh"
#include "sodium/paint.hh"
#include "sodium/theme.hh"

#include "shaders_generated.hh"

namespace zod::sodium {

Cx S = {};
DrawData* dcx = nullptr;

static constexpr auto PI = 3.14159265358979323846f;

auto resize(vec2 size) -> void {
  dcx->projection = ortho(0.f, size.x, size.y, 0.f, -1.f, 1.f);
  dcx->framebuffer->resize(size.x, size.y);
}

auto rect_(PaintCx& pcx, vec4 col, vec2 a, vec2 b) -> void {
  auto& draw_list = pcx.active_draw_list();
  auto start = draw_list.draw_data.size();

  auto a0 = vec2(a.x, a.y + b.y);
  auto a1 = a + b;
  auto a2 = vec2(a.x + b.x, a.y);
  auto a3 = a;

  draw_list.draw_data.push_back({ a0, { 0, 0 } });
  draw_list.draw_data.push_back({ a1, { 1, 0 } });
  draw_list.draw_data.push_back({ a2, { 1, 1 } });
  draw_list.draw_data.push_back({ a3, { 0, 1 } });

  draw_list.indices.push_back(start);
  draw_list.indices.push_back(start + 1);
  draw_list.indices.push_back(start + 2);
  draw_list.indices.push_back(start + 2);
  draw_list.indices.push_back(start + 3);
  draw_list.indices.push_back(start);

  draw_list.colors.push_back(col);
  draw_list.colors.push_back(col);
}

auto push_triangle(PaintCx& pcx, vec4 color, vec2 a, vec2 b, vec2 c) -> void {
  auto& draw_list = pcx.active_draw_list();
  auto start = draw_list.draw_data.size();
  draw_list.draw_data.push_back({ a, { 0, 0 } });
  draw_list.draw_data.push_back({ b, { 0, 0 } });
  draw_list.draw_data.push_back({ c, { 0, 0 } });
  draw_list.indices.push_back(start);
  draw_list.indices.push_back(start + 1);
  draw_list.indices.push_back(start + 2);
  draw_list.colors.push_back(color);
}

auto has_rounding_flag(RoundingFlags flags, RoundingFlags flag) -> bool {
  return (flags & flag) != 0;
}

auto append_point(Vector<vec2>& points, vec2 point) -> void {
  if (points.empty() or points.back().x != point.x or
      points.back().y != point.y) {
    points.push_back(point);
  }
}

auto append_corner(Vector<vec2>& points, vec2 center, f32 radius,
                   f32 start_angle, f32 end_angle) -> void {
  constexpr auto segments = 6;
  for (auto i = 0; i <= segments; ++i) {
    auto t = f32(i) / f32(segments);
    auto angle = start_angle + (end_angle - start_angle) * t;
    append_point(points,
                 center + vec2(glm::cos(angle), glm::sin(angle)) * radius);
  }
}

///////////////////////////////////////////////////////////
/// Rect //////////////////////////////////////////////////
///////////////////////////////////////////////////////////
auto rect::paint(PaintCx& pcx) const -> void {
  const auto position = inner.position;
  const auto size = inner.size;
  auto padded_position = position + padding;
  auto padded_size = glm::max(size - padding * 2.0f, vec2(0.0f));
  if (padded_size.x <= 0.0f or padded_size.y <= 0.0f) {
    return;
  }

  auto radius =
      std::min(rounding, std::min(padded_size.x, padded_size.y) * 0.5f);
  if (radius <= 0.0f or flags == RECT_ROUND_NONE) {
    rect_(pcx, color, padded_position, padded_size);
    return;
  }

  auto left = padded_position.x;
  auto top = padded_position.y;
  auto right = padded_position.x + padded_size.x;
  auto bottom = padded_position.y + padded_size.y;
  auto points = Vector<vec2>();

  if (has_rounding_flag(flags, RECT_ROUND_TOP_LEFT)) {
    append_point(points, { left + radius, top });
  } else {
    append_point(points, { left, top });
  }

  if (has_rounding_flag(flags, RECT_ROUND_TOP_RIGHT)) {
    append_point(points, { right - radius, top });
    append_corner(points, { right - radius, top + radius }, radius, -PI * 0.5f,
                  0.0f);
  } else {
    append_point(points, { right, top });
  }

  if (has_rounding_flag(flags, RECT_ROUND_BOTTOM_RIGHT)) {
    append_point(points, { right, bottom - radius });
    append_corner(points, { right - radius, bottom - radius }, radius, 0.0f,
                  PI * 0.5f);
  } else {
    append_point(points, { right, bottom });
  }

  if (has_rounding_flag(flags, RECT_ROUND_BOTTOM_LEFT)) {
    append_point(points, { left + radius, bottom });
    append_corner(points, { left + radius, bottom - radius }, radius, PI * 0.5f,
                  PI);
  } else {
    append_point(points, { left, bottom });
  }

  if (has_rounding_flag(flags, RECT_ROUND_TOP_LEFT)) {
    append_point(points, { left, top + radius });
    append_corner(points, { left + radius, top + radius }, radius, PI,
                  PI * 1.5f);
  }

  auto center = padded_position + padded_size * 0.5f;
  for (auto i = usize(0); i < points.size(); ++i) {
    push_triangle(pcx, color, center, points[i],
                  points[(i + 1) % points.size()]);
  }
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

static auto append_draw_list(DrawList& target, const DrawList& source) -> void {
  auto draw_data_offset = u32(target.draw_data.size());
  target.draw_data.insert(target.draw_data.end(), source.draw_data.begin(),
                          source.draw_data.end());
  target.colors.insert(target.colors.end(), source.colors.begin(),
                       source.colors.end());

  target.indices.reserve(target.indices.size() + source.indices.size());
  for (auto index : source.indices) {
    target.indices.push_back(index + draw_data_offset);
  }
}

auto submit(PaintCx& cx, SharedPtr<GPUTexture> texture) -> void {
  append_draw_list(cx.draw_list, cx.foreground_draw_list);

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
  dcx->shader->uniform_float("u_projection", value_ptr(dcx->projection), 16);
  dcx->framebuffer->bind();
  GPUState::get().set_blend(Blend::Alpha);
  dcx->batch->draw(dcx->shader, cx.draw_list.indices.size());
  GPUState::get().set_blend(Blend::None);
  dcx->framebuffer->unbind();

  cx.draw_list.draw_data.clear();
  cx.draw_list.colors.clear();
  cx.draw_list.indices.clear();
  cx.foreground_draw_list.draw_data.clear();
  cx.foreground_draw_list.colors.clear();
  cx.foreground_draw_list.indices.clear();
  cx.paint_foreground = false;
}

auto present() -> void {
  dcx->present->bind();
  dcx->framebuffer->get_slot(0)->bind();
  dcx->present->uniform_int("u_texture", ADDR(0));
  GPUState::get().draw_immediate(3);
}

auto submit(PaintCx& cx) -> void { submit(cx, dcx->white_texture); }

auto flush_foreground(PaintCx& cx) -> void {
  if (cx.deferred_foreground_paints.empty() and
      cx.foreground_draw_list.indices.empty() and
      cx.foreground_text_draw_list.positions.empty()) {
    return;
  }

  auto foreground_cx = PaintCx();
  {
    auto foreground = ScopedForegroundPaint(foreground_cx);
    for (const auto& paint : cx.deferred_foreground_paints) {
      paint(foreground_cx);
    }
  }

  append_draw_list(foreground_cx.draw_list, cx.foreground_draw_list);
  submit(foreground_cx);
  flush_icons();
  draw_images();
  Font::get().submit(foreground_cx);

  cx.deferred_foreground_paints.clear();
  cx.foreground_draw_list.draw_data.clear();
  cx.foreground_draw_list.colors.clear();
  cx.foreground_draw_list.indices.clear();
  cx.foreground_text_draw_list.clear();
}

} // namespace zod::sodium
