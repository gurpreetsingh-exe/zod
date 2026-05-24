#include "sodium/gui.hh"
#include "gpu/backend.hh"
#include "sodium/paint.hh"
#include "sodium/theme.hh"

#include "shaders_generated.hh"

namespace zod::sodium {

DrawData* dcx = nullptr;

auto resize(vec2 size) -> void {
  dcx->projection = ortho(0.f, size.x, size.y, 0.f, -1.f, 1.f);
  dcx->framebuffer->resize(size.x, size.y);
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
  dcx->shader->uniform_float("u_projection", value_ptr(dcx->projection), 16);
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

} // namespace zod::sodium
