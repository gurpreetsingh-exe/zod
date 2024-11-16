#include "context.hh"
#include "backend.hh"
#include "platform.hh"
#include "timer.hh"

#ifdef OPENGL_BACKEND
#include "opengl/backend.hh"
#endif
#ifdef VULKAN_BACKEND
#include "vulkan/backend.hh"
#endif

namespace zod {

static Shared<GPUContext> g_active_context = nullptr;
static Unique<GPUBackend> g_backend = nullptr;
static Shared<GPUState> g_state = nullptr;
static Unique<GPUTimer> g_timer = nullptr;

#ifdef OPENGL_BACKEND
static GPUBackendType g_backend_type = GPUBackendType::OpenGL;
#endif
#ifdef VULKAN_BACKEND
static GPUBackendType g_backend_type = GPUBackendType::Vulkan;
#endif

GPUContext::GPUContext() { m_active = false; }

auto GPUContext::get() -> Shared<GPUContext> { return g_active_context; }

static auto gpu_backend_create() -> GPUBackend& {
  ZASSERT(g_backend == nullptr);
#ifdef OPENGL_BACKEND
  g_backend = unique<GLBackend>();
#endif
#ifdef VULKAN_BACKEND
  g_backend = unique<VKBackend>();
#endif
  return *g_backend;
}

auto gpu_context_create(void* glfw_window) -> Shared<GPUContext> {
  auto& backend = gpu_backend_create();
  auto context = backend.create_context(glfw_window);
  gpu_context_active_set(context);
  g_state = backend.create_state();
  g_timer = unique<GPUTimer>();
  return context;
}

auto gpu_context_active_set(Shared<GPUContext> context) -> void {
  if (not context) {
    return;
  }
  g_active_context = context;
}

auto gpu_context_active_get() -> Shared<GPUContext> { return g_active_context; }

auto GPUBackend::get() -> GPUBackend& { return *g_backend; }
auto GPUState::get() -> GPUState& { return *g_state; }
auto GPUTimer::get() -> GPUTimer& { return *g_timer; }

} // namespace zod
