#pragma once

#include "framebuffer.hh"

namespace zod {

class GPUContext {
public:
  Unique<GPUFrameBuffer> active_fb;

protected:
  bool m_active;
  void* m_window;

public:
  GPUContext();
  virtual ~GPUContext() = default;

  static auto get() -> Shared<GPUContext>;
};

auto gpu_context_create(void* glfw_window) -> Shared<GPUContext>;
auto gpu_context_active_set(Shared<GPUContext>) -> void;
auto gpu_context_active_get() -> Shared<GPUContext>;

} // namespace zod
