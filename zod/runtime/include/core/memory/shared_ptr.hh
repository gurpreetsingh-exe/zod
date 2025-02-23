#pragma once

#include <memory>

namespace zod {

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename T, typename... Args>
[[always_inline]] auto shared(Args&&... args) -> SharedPtr<T> {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace zod
