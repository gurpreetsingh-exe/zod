#pragma once

#include <vector>

namespace zod {

template <class T, class Allocator = std::allocator<T>>
using Vector = std::vector<T, Allocator>;

} // namespace zod
