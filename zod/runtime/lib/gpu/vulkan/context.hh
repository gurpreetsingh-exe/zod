#pragma once

#include "gpu/context.hh"

namespace zod {

class VKContext : public GPUContext {
public:
  VKContext(void* /* glfw_window */);
  ~VKContext();

public:
  static auto get() -> SharedPtr<VKContext> {
    return static_pointer_cast<VKContext>(GPUContext::get());
  }
};

} // namespace zod
