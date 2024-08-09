#pragma once

#include "texture.hh"

namespace zod {

enum GPUAttachmentType : int {
  Depth = 0,
  DepthStencil,
  Color0,
  Color1,
  Color2,
  Color3,
  Color4,
  Color5,
  Color6,
  Color7,
  ColorMax,
};

constexpr int GPU_FB_MAX_COLOR_ATTACHMENT =
    GPUAttachmentType::ColorMax - GPUAttachmentType::Color0;

struct GPUAttachment {
  Shared<GPUTexture> texture;
};

class GPUFrameBuffer {
protected:
  i32 m_width = 0;
  i32 m_height = 0;
  std::vector<GPUAttachment> m_color_attachments;
  i32 m_samples = 1;
  // i32 m_viewport[4] = { 0 };
  // i32 m_scissor[4] = { 0 };
  GPUFrameBuffer(i32 width, i32 height) : m_width(width), m_height(height) {}

public:
  virtual ~GPUFrameBuffer() = default;
  virtual auto bind() -> void = 0;
  virtual auto unbind() -> void = 0;
  virtual auto resize(i32, i32) -> void = 0;
  virtual auto check() -> void = 0;
  virtual auto clear() -> void = 0;
  virtual auto add_color_attachment(GPUAttachment&) -> void = 0;
  virtual auto add_depth_attachment() -> void = 0;
  auto get_slot(usize slot) -> GPUAttachment& {
    return m_color_attachments[slot];
  }
  auto get_width() -> i32 { return m_width; }
  auto get_height() -> i32 { return m_height; }
};

} // namespace zod
