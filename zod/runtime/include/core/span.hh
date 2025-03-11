#pragma once

#include <initializer_list>

namespace zod {

template <typename T>
class Span {
public:
  Span() = default;
  Span(T* arr, usize size) : m_inner(arr), m_size(size) {}

public:
  auto data() const -> T* { return m_inner; }
  auto size() const -> usize { return m_size; }
  auto size_bytes() const -> usize { return m_size * sizeof(T); }

  auto begin() const -> const T* { return m_inner; }
  auto end() const -> const T* { return m_inner + m_size; }

  auto operator[](usize idx) const -> T {
    ZASSERT(idx < m_size);
    return m_inner[idx];
  }

private:
  T* m_inner = nullptr;
  usize m_size = 0;
};

} // namespace zod
