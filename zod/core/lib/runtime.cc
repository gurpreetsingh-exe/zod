#include "runtime.hh"

namespace zod {

static Runtime* g_runtime = nullptr;

auto Runtime::init() -> void {
  ZASSERT(not g_runtime, "runtime already initialized");
  g_runtime = new Runtime();
}

auto Runtime::destroy() -> void {
  delete g_runtime;
}

} // namespace zod
