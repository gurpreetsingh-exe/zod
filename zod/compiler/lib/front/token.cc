#include "front/token.hh"

namespace zod {

auto format_as(TokenKind kind) -> std::string {
  switch (kind) {
    case TokenKind::Eq:
      return "=";
    case TokenKind::Lt:
      return "<";
    case TokenKind::Le:
      return "<=";
    case TokenKind::EqEq:
      return "==";
    case TokenKind::Ne:
      return "!=";
    case TokenKind::Ge:
      return ">=";
    case TokenKind::Gt:
      return ">";
    case TokenKind::AndAnd:
      return "&&";
    case TokenKind::OrOr:
      return "||";
    case TokenKind::Not:
      return "!";
    case TokenKind::Tilde:
      return "~";
    case TokenKind::At:
      return "@";
    case TokenKind::Dot:
      return ".";
    case TokenKind::DotDot:
      return "..";
    case TokenKind::DotDotDot:
      return "...";
    case TokenKind::DotDotEq:
      return "..=";
    case TokenKind::Comma:
      return ",";
    case TokenKind::Semi:
      return ";";
    case TokenKind::Colon:
      return ":";
    case TokenKind::ModSep:
      return "::";
    case TokenKind::RArrow:
      return "->";
    case TokenKind::LArrow:
      return "<-";
    case TokenKind::FatArrow:
      return "=>";
    case TokenKind::Pound:
      return "#";
    case TokenKind::Dollar:
      return "$";
    case TokenKind::Question:
      return "?";
    case TokenKind::Ident:
      return "Ident";
    case TokenKind::DocCommentInner:
      return "//!";
    case TokenKind::DocCommentOuter:
      return "///";
    case TokenKind::Eof:
      return "<eof>";
#define X(bin_op, op)                                                          \
  case TokenKind::bin_op:                                                      \
    return #op;
      BINARY_OPS()
#undef X

#define X(bin_op, op)                                                          \
  case TokenKind::bin_op##Eq:                                                  \
    return CAT(op, =);
      BINARY_OPS()
#undef X

#define X(delim, o, _)                                                         \
  case TokenKind::Open##delim:                                                 \
    return o;
      DELIMITERS()
#undef X

#define X(delim, _, c)                                                         \
  case TokenKind::Close##delim:                                                \
    return c;
      DELIMITERS()
#undef X

#define X(lit)                                                                 \
  case TokenKind::Lit##lit:                                                    \
    return #lit;
      LITERALS()
#undef X
  }

  UNREACHABLE();
}

} // namespace zod
