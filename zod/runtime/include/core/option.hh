#pragma once

namespace zod {

template <class T>
class Option {
public:
  explicit Option() : m_inner(T()), m_has_value(false) {}
  Option(const T& value) : m_inner(std::move(value)), m_has_value(true) {}
  operator bool() { return m_has_value; }
  auto operator*() -> T& {
    ZASSERT(m_has_value);
    return m_inner;
  }

private:
  bool m_has_value;
  T m_inner = {};
};

template <class T>
auto some(const T& v) -> Option<T> {
  return Option(std::move(v));
}

template <class T>
auto none() -> Option<T> {
  return Option<T>();
}

} // namespace zod
