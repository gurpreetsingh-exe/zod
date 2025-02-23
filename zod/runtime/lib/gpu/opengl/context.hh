#pragma once

#include "gpu/context.hh"

namespace zod {

class GLContext : public GPUContext {
public:
  GLContext(void* /* glfw_window */);
  ~GLContext();

public:
  static auto get() -> SharedPtr<GLContext> {
    return static_pointer_cast<GLContext>(GPUContext::get());
  }
};

} // namespace zod
