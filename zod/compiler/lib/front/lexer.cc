#include "front/lexer.hh"

namespace zod {

#define IS_ID_START(c)                                                         \
  ((c) == '_' or ('a' <= (c) and (c) <= 'z') or ('A' <= (c) and (c) <= 'Z') or \
   ((c) > '\x7f'))

static auto is_whitespace(char c) -> bool {
  switch (c) {
      // Usual ASCII suspects
    case '\t':
    case '\n':
    case '\x0b': // vertical tab
    case '\x0c': // form feed
    case '\r':
    case ' ':

      // NEXT LINE from latin1
      // case '\x85':
      //   // Bidi markers
      // case L'\u200E': // LEFT-TO-RIGHT MARK
      // case L'\u200F': // RIGHT-TO-LEFT MARK
      //
      //   // Dedicated whitespace characters from Unicode
      // case L'\u2028': // LINE SEPARATOR
      // case L'\u2029': // PARAGRAPH SEPARATOR

      return true;
  }
  return false;
}

struct Lexer {
  usize index = 0;
  char chr = 0;
  char prev = 0;
  bool precededByWhitespace = false;
  const std::string& Src;

  Lexer(const std::string& src) : Src(src) {
    if (not Src.empty()) {
      chr = Src[index];
    }
  }

  auto bump() -> void {
    prev = chr;
    if (index < Src.size() - 1) {
      chr = Src[++index];
    } else {
      ++index;
      chr = 0;
    }
  }

  auto first() -> char { return index < Src.size() - 2 ? Src[index + 1] : 0; }

  auto symbol(usize start) -> Symbol {
    auto string = std::string_view(Src.data() + start, index - start);
    return Symbol::intern(string);
  }

  auto eat_identifier() -> void {
    if (not IS_ID_START(chr)) {
      return;
    }

    bump();
    while (isalnum(chr) or chr == '_') { bump(); }
  }

  auto eat_decimal_digits() -> bool {
    bool has_digits = false;

    while (true) {
      if (chr == '_') {
        bump();
        continue;
      }
      if (isdigit(chr)) {
        has_digits = true;
        bump();
        continue;
      }
      break;
    }

    return has_digits;
  }

  auto number(Base& base) -> TokenKind {
    if (chr == '0') {
      bump();
      switch (chr) {
        case 'b':
          base = Binary;
          TODO();
        case 'o':
          TODO();
        case 'x':
          TODO();
        case '0' ... '9':
        case '_':
          eat_decimal_digits();
          break;
        case '.':
        case 'e':
        case 'E':
          break;
        default:
          return TokenKind::LitInteger;
      }
    } else {
      eat_decimal_digits();
    }

    if (chr == '.' and first() != '.') {
      bump();
      TODO();
    } else if (chr == 'e' or chr == 'E') {
      TODO();
    } else {
      return TokenKind::LitInteger;
    }
    UNREACHABLE();
  }

  auto single_quoted_string() -> bool {
    ZASSERT(prev == '\'');
    if (first() == '\'' && chr != '\\') {
      bump();
      bump();
      return true;
    }

    while (true) {
      switch (chr) {
        case '\'': {
          bump();
          return true;
        }
        case '/':
          goto end;
        case '\n': {
          if (first() != '\'')
            goto end;
        } break;
        case '\0': {
          if (chr == '\0')
            goto end;
        } break;
        case '\\': {
          bump();
          bump();
        } break;
        default: {
          bump();
        } break;
      }
    }
  end:
    return false;
  }

  auto double_quoted_string() -> bool {
    ZASSERT(prev == '"');
    while (true) {
      char c = chr;
      bump();
      if (c == '"') {
        return true;
      } else if (c == '\\' and (chr == '\\' or chr == '"')) {
        bump();
      }
    }
    return false;
  }

  auto character(Symbol* sym, usize start) -> TokenKind {
    ZASSERT(prev == '\'');
    bool terminated = single_quoted_string();
    if (terminated) {
      *sym = symbol(start);
      return TokenKind::LitChar;
    }
    return TokenKind::Err;
  }

  auto line_comment() -> Option<TokenKind> {
    bump();
    Option<TokenKind> kind = {};

    if (chr == '!') {
      // `//!` inner doc comment
      kind = TokenKind::DocCommentInner;
    } else if (chr == '/' and first() != '/') {
      // `///` is an outer doc comment, `////` (more than 3 slashes) is not
      kind = TokenKind::DocCommentOuter;
    }

    while (chr != '\n') { bump(); }
    return kind;
  }

  auto next() -> Option<Token> {
    while (true) {
      usize start = index;
      Option<TokenKind> kind = {};
      Symbol sym = { 0 };

      switch (chr) {
        case ';': {
          kind = TokenKind::Semi;
          bump();
        } break;
        case ',': {
          kind = TokenKind::Comma;
          bump();
        } break;
        case '.': {
          kind = TokenKind::Dot;
          bump();
        } break;
        case '(': {
          kind = TokenKind::OpenParen;
          bump();
        } break;
        case ')': {
          kind = TokenKind::CloseParen;
          bump();
        } break;
        case '{': {
          kind = TokenKind::OpenBrace;
          bump();
        } break;
        case '}': {
          kind = TokenKind::CloseBrace;
          bump();
        } break;
        case '[': {
          kind = TokenKind::OpenBracket;
          bump();
        } break;
        case ']': {
          kind = TokenKind::CloseBracket;
          bump();
        } break;
        case '@': {
          kind = TokenKind::At;
          bump();
        } break;
        case '#': {
          kind = TokenKind::Pound;
          bump();
        } break;
        case '~': {
          kind = TokenKind::Tilde;
          bump();
        } break;
        case '?': {
          kind = TokenKind::Question;
          bump();
        } break;
        case ':': {
          kind = TokenKind::Colon;
          bump();
          if (chr == ':') {
            kind = TokenKind::ModSep;
            bump();
          }
        } break;
        case '$': {
          kind = TokenKind::Dollar;
          bump();
        } break;
        case '=': {
          kind = TokenKind::Eq;
          bump();
        } break;
        case '!': {
          kind = TokenKind::Not;
          bump();
        } break;
        case '<': {
          kind = TokenKind::Lt;
          bump();
        } break;
        case '>': {
          kind = TokenKind::Gt;
          bump();
        } break;
        case '-': {
          kind = TokenKind::Minus;
          bump();
          if (chr == '>') {
            kind = TokenKind::RArrow;
            bump();
          }
        } break;
        case '&': {
          kind = TokenKind::And;
          bump();
        } break;
        case '|': {
          kind = TokenKind::Or;
          bump();
        } break;
        case '+': {
          kind = TokenKind::Plus;
          bump();
        } break;
        case '*': {
          kind = TokenKind::Star;
          bump();
        } break;
        case '/': {
          bump();
          if (chr == '/') {
            kind = line_comment();
            if (not kind.has_value()) {
              precededByWhitespace = true;
              continue;
            }
            sym = symbol(start);
            return Token(kind.value(), Span(start, index), sym);
          } else if (chr == '*') {
            eprintln("block comments are not supported");
            kind = line_comment();
            if (not kind.has_value()) {
              precededByWhitespace = true;
              continue;
            }
            sym = symbol(start);
            return Token(kind.value(), Span(start, index), sym);
          }
          kind = TokenKind::Slash;
        } break;
        case '^': {
          kind = TokenKind::Caret;
          bump();
        } break;
        case '%': {
          kind = TokenKind::Percent;
          bump();
        } break;
        case '\'': {
          bump();
          kind = character(&sym, start);
          return Token(kind.value(), Span(start, index), sym);
        } break;
        case '"': {
          bump();
          bool terminated = double_quoted_string();
          // usize suffix_start = index;
          kind = TokenKind::LitStr;
          if (terminated) {
            sym = symbol(start);
          } else {
            kind = TokenKind::Err;
          }
          return Token(kind.value(), Span(start, index), sym);
        } break;
        case '\0': {
          kind = TokenKind::Eof;
        } break;
        default: {
          if (isalpha(chr) or chr == '_') {
            eat_identifier();
            sym = symbol(start);
            return Token(TokenKind::Ident, Span(start, index), sym);
          } else if (isdigit(chr)) {
            Base base = Base::Decimal;
            kind = number(base);
            sym = symbol(start);
            return Token(kind.value(), Span(start, index), sym);
          } else if (is_whitespace(chr)) {
            precededByWhitespace = true;
            bump();
            continue;
          }
        } break;
      }

      ZASSERT(kind.has_value(), "({}, {:d})", chr, chr);
      return Token(kind.value(), Span(start, index), sym);
    }
  }
};

auto lex_file(const std::string& src) -> std::vector<Token> {
  auto tokens = std::vector<Token>();
  auto lexer = Lexer(src);
  while (true) {
    auto opt_token = lexer.next();
    if (not opt_token.has_value()) {
      break;
    }
    auto token = opt_token.value();
    if (token.kind == TokenKind::Eof) {
      break;
    }
    fmt::println("{}", token);
    tokens.push_back(token);
  }
  return tokens;
}

} // namespace zod
