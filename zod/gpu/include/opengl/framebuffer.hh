#pragma once

#include <glad/glad.h>

#include "../framebuffer.hh"
#include "opengl/texture.hh"

namespace zod {

class GLFrameBuffer : public GPUFrameBuffer {
  friend class GLTexture;

private:
  GLuint m_id;

public:
  GLFrameBuffer(i32 /* width */, i32 /* height */);
  ~GLFrameBuffer();

  auto bind() -> void override;
  auto unbind() -> void override;
  auto resize(i32, i32) -> void override;
  auto check() -> void override;
  auto add_color_attachment(GPUAttachment&) -> void override;
};

} // namespace zod
