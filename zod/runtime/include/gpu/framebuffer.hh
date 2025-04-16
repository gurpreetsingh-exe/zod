#pragma once

#include "gpu/texture.hh"

namespace zod {

enum class GPUAttachmentType : int {
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
    int(GPUAttachmentType::ColorMax) - int(GPUAttachmentType::Color0);

class GPUFrameBuffer {
protected:
  i32 m_width = 0;
  i32 m_height = 0;
  Vector<SharedPtr<GPUTexture>> m_color_attachments;
  // SharedPtr<GPUTexture> m_depth_attachment = nullptr;
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
  virtual auto clear_color(vec4) -> void = 0;
  virtual auto add_color_attachment(SharedPtr<GPUTexture>) -> void = 0;
  virtual auto add_depth_attachment() -> void = 0;
  virtual auto read_pixel(usize, i32, i32) -> u32 = 0;
  auto get_slot(usize slot) -> SharedPtr<GPUTexture> {
    return m_color_attachments[slot];
  }
  virtual auto bind_depth(usize /* slot */) -> void = 0;
  // auto get_depth() -> GPUAttachment& {
  //   ZASSERT(m_depth_attachment.get() != nullptr);
  //   return *m_depth_attachment;
  // }
  auto get_width() -> i32 { return m_width; }
  auto get_height() -> i32 { return m_height; }
};

} // namespace zod
