#pragma once

namespace zod {

struct Span {
  u32 lo;
  u32 hi;

  Span() = default;
  Span(u32 l, u32 h) : lo(l), hi(h) {}
};

} // namespace zod
