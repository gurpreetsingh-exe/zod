#include "context.hh"
#include "backend.hh"
#include "platform.hh"
#include "renderer.hh"

#include "opengl/backend.hh"

namespace zod {

static Shared<GPUContext> g_active_context = nullptr;
static Unique<GPUBackend> g_backend = nullptr;
static Shared<GPURenderer> g_renderer = nullptr;
static Shared<GPUState> g_state = nullptr;
static GPUBackendType g_backend_type = GPUBackendType::OpenGL;

GPUContext::GPUContext() { m_active = false; }

auto GPUContext::get() -> Shared<GPUContext> { return g_active_context; }

static auto gpu_backend_create() -> GPUBackend& {
  ZASSERT(g_backend == nullptr);
  g_backend = unique<GLBackend>();
  return *g_backend;
}

auto gpu_context_create(void* glfw_window) -> Shared<GPUContext> {
  auto& backend = gpu_backend_create();
  auto context = backend.create_context(glfw_window);
  gpu_context_active_set(context);
  g_renderer = backend.create_renderer();
  g_state = backend.create_state();
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
auto GPURenderer::get() -> GPURenderer& { return *g_renderer; }
auto GPUState::get() -> GPUState& { return *g_state; }

} // namespace zod
