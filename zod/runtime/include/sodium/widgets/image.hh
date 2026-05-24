#pragma once

#include "sodium/widgets/widget.hh"

namespace zod {
class GPUTexture;
} // namespace zod

namespace zod::sodium {

class Image : public Widget {
public:
  Image() = default;
  explicit Image(SharedPtr<GPUTexture> image) : m_image(image) {}
  explicit Image(vec4 color) { set_image(color); }

  auto set_image(vec4 color) -> void { m_style.background = color; }
  auto set_accessible_text(String text) -> void {
    m_accessible_text = std::move(text);
  }

  auto compute_desired_size(vec2) -> vec2 override;
  auto arrange(const Rect&) -> void override;
  auto paint(PaintCx&) const -> void override;

private:
  String m_accessible_text;
  SharedPtr<GPUTexture> m_image = nullptr;
};

} // namespace zod::sodium
