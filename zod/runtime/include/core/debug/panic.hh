#pragma once

#include "core/backtrace.hh"
#include <experimental/source_location>

namespace zod {

using Location = std::experimental::source_location;

template <typename... T>
auto __panic(Location loc, fmt::format_string<T...> fmt, T&&... args) {
  fmt::print(stderr, "panic at {}:{}", loc.file_name(), loc.line());
  fmt::println(stderr, fmt, args...);
  fmt::println(stderr, "BACKTRACE:");
  print_backtrace();
  exit(1);
}

} // namespace zod
