#pragma once

#include "gpu/framebuffer.hh"

namespace zod {

class GPUContext {
public:
  UniquePtr<GPUFrameBuffer> active_fb;

protected:
  bool m_active;
  void* m_window;

public:
  GPUContext();
  virtual ~GPUContext() = default;

  static auto get() -> SharedPtr<GPUContext>;
};

auto gpu_context_create(void* glfw_window) -> SharedPtr<GPUContext>;
auto gpu_context_active_set(SharedPtr<GPUContext>) -> void;
auto gpu_context_active_get() -> SharedPtr<GPUContext>;

} // namespace zod
