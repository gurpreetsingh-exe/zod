#include "session/args.hh"
#include "session/session.hh"

auto main(int argc, char** argv) -> int {
  auto config = zod::config_from_args(argc, argv);
  auto sess = zod::Session(config);
  sess.run_compiler();
}
