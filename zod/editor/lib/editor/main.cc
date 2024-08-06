#include "application/context.hh"

using namespace zod;

auto main() -> int {
  ZCtxt::create();
  ZCtxt::get().run();
  ZCtxt::drop();
}
