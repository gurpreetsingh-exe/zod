#include "engine/runtime.hh"
#include "engine/registry.hh"

namespace zod {

static Runtime* g_runtime = nullptr;

Runtime::Runtime() {
  m_scene = UUID();
  AssetRegistry::reg(m_scene, shared<Scene>());
}

auto Runtime::get() -> Runtime& { return *g_runtime; }
auto Runtime::scene() -> Scene& {
  return *AssetRegistry::get<SharedPtr<Scene>>(m_scene);
}

auto Runtime::init() -> void {
  ZASSERT(not g_runtime, "runtime already initialized");
  g_runtime = new Runtime();
}

auto Runtime::destroy() -> void { delete g_runtime; }

} // namespace zod
