#pragma once

namespace zod {

/// Used in ADDR() macro
template <typename T>
struct Addr {
  Addr(T v) : inner(v) {}
  auto operator*() const -> const T* { return &inner; }
  T inner;
};

} // namespace zod
