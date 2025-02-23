#pragma once

#include <memory>

namespace zod {

template <class T, class Deleter = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, Deleter>;

template <typename T, typename... Args>
[[always_inline]] auto unique(Args&&... args) -> UniquePtr<T> {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace zod
