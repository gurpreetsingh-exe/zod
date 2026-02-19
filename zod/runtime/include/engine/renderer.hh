#pragma once

#include "engine/entity.hh"
#include "gpu/backend.hh"

namespace zod {

class Renderer {
public:
  Renderer();
  virtual ~Renderer() = default;

public:
  virtual auto tick() -> void = 0;
  virtual auto resize(f32, f32) -> void;

public:
  auto get_render_target() const -> SharedPtr<GPUFrameBuffer> {
    return m_framebuffer;
  }

protected:
  SharedPtr<GPUFrameBuffer> m_framebuffer = nullptr;
};

class ForwardRenderer : public Renderer {
public:
  ForwardRenderer();
  ~ForwardRenderer() = default;

public:
  auto tick() -> void override;
};

class DeferredRenderer : public Renderer {
public:
  DeferredRenderer();
  ~DeferredRenderer() = default;

public:
  auto tick() -> void override;
  auto resize(f32, f32) -> void override;

private:
  SharedPtr<GPUFrameBuffer> m_gbuffer = nullptr;
};

} // namespace zod
