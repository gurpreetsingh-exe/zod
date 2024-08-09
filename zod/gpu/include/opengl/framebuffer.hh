#pragma once

#include <glad/glad.h>

#include "../framebuffer.hh"
#include "opengl/texture.hh"

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
  auto add_color_attachment(GPUAttachment&) -> void override;
  auto add_depth_attachment() -> void override;
};

} // namespace zod
