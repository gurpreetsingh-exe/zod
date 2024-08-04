#pragma once

#include "span/span.hh"
#include "span/symbol.hh"

namespace zod {

#define CAT(a, b) #a #b
#define CAT2(a, b) a##b
#define STR(a) #a

#define BINARY_OPS()                                                           \
  X(Plus, +)                                                                   \
  X(Minus, -)                                                                  \
  X(Star, *)                                                                   \
  X(Slash, /)                                                                  \
  X(Percent, %)                                                                \
  X(Caret, ^)                                                                  \
  X(And, &)                                                                    \
  X(Or, |)                                                                     \
  X(Shl, <)                                                                    \
  X(Shr, >)

#define DELIMITERS()                                                           \
  X(Paren, "(", ")")                                                           \
  X(Brace, "{", "}")                                                           \
  X(Bracket, "[", "]")                                                         \
  X(Invisible, "0", "0")

#define LITERALS()                                                             \
  X(Bool)                                                                      \
  X(Byte)                                                                      \
  X(Char)                                                                      \
  X(Integer)                                                                   \
  X(Float)                                                                     \
  X(Str)

enum class TokenKind {
  /// Expression-operator symbols.
  Eq,
  Lt,
  Le,
  EqEq,
  Ne,
  Ge,
  Gt,
  AndAnd,
  OrOr,
  Not,
  Tilde,

#define X(bin_op, _) bin_op,
  /// Binary operators `+`, `*`
  BINARY_OPS()
#undef X

#define X(bin_op, _) bin_op##Eq,
  /// Binary eq operators `+=`, `*=`
  BINARY_OPS()
#undef X

#define X(delim, o, c) Open##delim,
  /// An opening delimiter (e.g., `{`).
  DELIMITERS()
#undef X

#define X(delim, o, c) Close##delim,
  /// A closing delimiter (e.g., `}`).
  DELIMITERS()
#undef X

#define X(lit) Lit##lit,
  /// Literals
  LITERALS()
#undef X

  /// Structural symbols
  At,
  Dot,
  DotDot,
  DotDotDot,
  DotDotEq,
  Comma,
  Semi,
  Colon,
  ModSep,
  RArrow,
  LArrow,
  FatArrow,
  Pound,
  Dollar,
  Question,

  Ident,
  /// A doc comment token `//!`.
  DocCommentInner,
  /// A doc comment token `///`.
  DocCommentOuter,
  Eof,
  Err,
};

/// Base of numeric literal encoding according to its prefix.
enum Base : u32 {
  /// Literal starts with "0b".
  Binary = 2,
  /// Literal starts with "0o".
  Octal = 8,
  /// Literal doesn't contain a prefix.
  Decimal = 10,
  /// Literal starts with "0x".
  Hexadecimal = 16,
};

struct Token {
  TokenKind kind;
  Span span;
  Symbol sym;
};

auto format_as(TokenKind kind) -> std::string;

} // namespace zod

template <>
struct fmt::formatter<zod::TokenKind> {
  constexpr auto parse(format_parse_context& ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(zod::TokenKind kind, format_context& ctx) const
      -> format_context::iterator {
    auto s = zod::format_as(kind);
    return fmt::format_to(ctx.out(), "{}", s);
  }
};

template <>
struct fmt::formatter<zod::Token> {
  constexpr auto parse(format_parse_context& ctx)
      -> format_parse_context::iterator {
    return ctx.begin();
  }

  auto format(const zod::Token& t, format_context& ctx) const
      -> format_context::iterator {
    std::string s;
    switch (t.kind) {
      default:
        break;
      case zod::TokenKind::Ident:
      case zod::TokenKind::DocCommentOuter:
      case zod::TokenKind::DocCommentInner:
#define X(lit) case zod::TokenKind::Lit##lit:
        LITERALS()
#undef X
        s = fmt::format(", symbol: \"{}\"", t.sym);
        break;
    }

    return fmt::format_to(ctx.out(),
                          "Token {{ kind: {}, span: {{ lo: {}, hi: {} }}{} }}",
                          t.kind, t.span.lo, t.span.hi, s);
  }
};
