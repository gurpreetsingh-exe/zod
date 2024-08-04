#include "session/session.hh"
#include "front/lexer.hh"
#include "io/obj.hh"
#include "span/file.hh"

namespace zod {

auto Session::run_compiler() -> void {
  g_Interner.fresh();
  // auto src = read_file(config.input);
  // lex_file(src);
  auto mesh = load_obj(config.input);
  delete mesh;
}

} // namespace zod
