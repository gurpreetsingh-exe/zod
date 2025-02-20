#pragma once

#define DISALLOW_COPY_AND_ASSIGN(T)                                            \
  T(T&&) = delete;                                                             \
  T(const T&) = delete;                                                        \
  T& operator=(const T&) = delete;                                             \
  T& operator=(T&&) = delete;

#define UNREACHABLE() __builtin_unreachable()

#define PANIC(...) (::zod::__panic(::zod::loc::current(), "\n    " __VA_ARGS__))

#define TODO()                                                                 \
  PANIC("TODO: not implemented in {}", __func__);                              \
  UNREACHABLE()

#define ZASSERT(cond, ...)                                                     \
  (((cond) ? static_cast<void>(0)                                              \
           : PANIC("assertion failed " #cond " " __VA_ARGS__)))

#define eprintln(...)                                                          \
  ::fmt::print(stderr, "\x1b[1;31merror\x1b[0m: ");                            \
  ::fmt::println(stderr, __VA_ARGS__);                                         \
  ::std::exit(1)

#define FMT(type, fmt_str, ...)                                                \
  template <>                                                                  \
  struct fmt::formatter<type> {                                                \
    constexpr auto parse(format_parse_context& ctx)                            \
        -> format_parse_context::iterator {                                    \
      return ctx.begin();                                                      \
    }                                                                          \
    auto format(type v, format_context& ctx) const                             \
        -> format_context::iterator {                                          \
      return fmt::format_to(ctx.out(), fmt_str, __VA_ARGS__);                  \
    }                                                                          \
  }

#define ADDR(...) *Addr(__VA_ARGS__)
