#pragma once

#include "engine/entity.hh"
#include "gpu/backend.hh"

namespace zod {

class Renderer {
public:
  Renderer();
  ~Renderer() = default;

public:
  auto tick() -> void;
  auto resize(f32, f32) -> void;

public:
  auto get_render_target() const -> SharedPtr<GPUFrameBuffer> {
    return m_framebuffer;
  }

private:
  SharedPtr<GPUFrameBuffer> m_framebuffer = nullptr;
};

} // namespace zod
