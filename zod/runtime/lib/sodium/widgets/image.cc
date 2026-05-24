#include "sodium/widgets/image.hh"
#include "sodium/gui.hh"

namespace zod::sodium {

struct TexturedRect {
  Rect rect;
  SharedPtr<GPUTexture> image = nullptr;
};

Vector<TexturedRect> g_textured_rects = {};

auto draw_images() -> void {
  auto cx = PaintCx();
  for (auto& r : g_textured_rects) {
    rect(r.rect, vec4(1.0f)).paint(cx);
    submit(cx, r.image);
  }
  g_textured_rects.clear();
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
}

} // namespace zod::sodium
