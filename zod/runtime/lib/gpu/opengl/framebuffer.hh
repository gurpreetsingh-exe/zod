#pragma once

#include <glad/glad.h>

#include "./texture.hh"
#include "gpu/framebuffer.hh"

namespace zod {

class GLFrameBuffer : public GPUFrameBuffer {
  friend class GLTexture;

private:
  GLuint m_id;
  GLuint m_depth_attachment = 0;
  GLint m_view[4];

public:
  GLFrameBuffer(i32 /* width */, i32 /* height */);
  ~GLFrameBuffer();

  auto bind() -> void override;
  auto unbind() -> void override;
  auto resize(i32, i32) -> void override;
  auto check() -> void override;
  auto clear() -> void override;
  auto clear_color(vec4) -> void override;
  auto add_color_attachment(GPUAttachment&) -> void override;
  auto add_depth_attachment() -> void override;

  auto read_pixel(usize idx, i32 x, i32 y) -> u32 override {
    u32 pixel;
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + idx);
    glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pixel);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return pixel;
  }
};

} // namespace zod
