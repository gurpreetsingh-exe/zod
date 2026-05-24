#include "sodium/icon.hh"
#include "gpu/backend.hh"
#include "sodium/gui.hh"
#include "sodium/paint.hh"

namespace zod::sodium {

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

} // namespace zod::sodium
