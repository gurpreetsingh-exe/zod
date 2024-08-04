#include "span/symbol.hh"

namespace zod {

Interner g_Interner = {};

namespace kw {
#define X(kw, unused) Symbol kw(__COUNTER__);
KEYWORDS()
#undef X
} // namespace kw

auto Symbol::intern(std::string_view symbol) -> Symbol {
  return g_Interner.intern(symbol);
}

auto Symbol::get() -> std::string_view { return g_Interner.strings[inner]; }

auto Interner::fresh() -> void {
#define X(unused, val)                                                         \
  do {                                                                         \
    std::string_view string = val;                                             \
    Symbol sym(u32(g_Interner.strings.size()));                                \
    g_Interner.strings.push_back(string);                                      \
    g_Interner.names[string] = sym;                                            \
  } while (0);
  KEYWORDS()
#undef X
}

auto Interner::clear() -> void {
  g_Interner.strings.clear();
  g_Interner.names.clear();
}

auto Interner::intern(std::string_view string) -> Symbol {
  if (g_Interner.names.contains(string)) {
    return g_Interner.names.at(string);
  }

  Symbol sym(u32(g_Interner.strings.size()));
  g_Interner.strings.push_back(string);
  g_Interner.names[string] = sym;
  return sym;
}

} // namespace zod
