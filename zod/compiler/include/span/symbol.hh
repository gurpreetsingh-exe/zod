#pragma once

namespace zod {

#define KEYWORDS()                                                             \
  X(Empty, "")                                                                 \
  X(PathRoot, "{{root}}")                                                      \
  X(Underscore, "_")                                                           \
  X(As, "as")                                                                  \
  X(Break, "break")                                                            \
  X(Const, "const")                                                            \
  X(Continue, "continue")                                                      \
  X(Else, "else")                                                              \
  X(Extern, "extern")                                                          \
  X(False, "false")                                                            \
  X(Fun, "fun")                                                                \
  X(For, "for")                                                                \
  X(If, "if")                                                                  \
  X(Impl, "impl")                                                              \
  X(In, "in")                                                                  \
  X(Let, "let")                                                                \
  X(Loop, "loop")                                                              \
  X(Match, "match")                                                            \
  X(Mod, "mod")                                                                \
  X(Move, "move")                                                              \
  X(Mut, "mut")                                                                \
  X(Pub, "pub")                                                                \
  X(Ref, "ref")                                                                \
  X(Return, "return")                                                          \
  X(SelfLower, "self")                                                         \
  X(SelfUpper, "Self")                                                         \
  X(Static, "static")                                                          \
  X(Struct, "struct")                                                          \
  X(Super, "super")                                                            \
  X(True, "true")                                                              \
  X(Type, "type")                                                              \
  X(Using, "using")                                                            \
  X(Where, "where")                                                            \
  X(While, "while")

struct Symbol {
  u32 inner;
  Symbol() = default;
  Symbol(u32 index) : inner(index) {}

  static auto intern(std::string_view string) -> Symbol;
  auto get() -> std::string_view;
};

struct Interner {
  std::vector<std::string_view> strings;
  std::unordered_map<std::string_view, Symbol> names;

  auto fresh() -> void;
  auto clear() -> void;
  auto intern(std::string_view string) -> Symbol;
};

/// Symbol interner
extern Interner g_Interner;

namespace kw {
#define X(kw, unused) extern Symbol kw;
KEYWORDS()
#undef X
} // namespace kw

} // namespace zod

template <>
struct fmt::formatter<zod::Symbol> {
  constexpr auto parse(format_parse_context& ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(zod::Symbol sym, format_context& ctx) const
      -> format_context::iterator {
    return fmt::format_to(ctx.out(), "{}{:d}", sym.get(), sym.inner);
  }
};
