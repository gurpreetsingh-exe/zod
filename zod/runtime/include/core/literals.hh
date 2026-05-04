#pragma once

#include "types.hh"

namespace zod {

constexpr auto operator""_kb(unsigned long long int n) -> usize {
  return n * 1024;
}

constexpr auto operator""_mb(unsigned long long int n) -> usize {
  return n * 1024_kb;
}

} // namespace zod
