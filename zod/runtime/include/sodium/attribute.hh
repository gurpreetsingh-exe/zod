#pragma once

#include "sodium/delegate.hh"

namespace zod::sodium {

template <class T>
class Attribute {
public:
  Attribute() = default;
  Attribute(T value) : m_value(std::move(value)) {}

  template <class Fn>
    requires std::is_invocable_r_v<T, Fn>
  Attribute(Fn callback) {
    bind(std::move(callback));
  }

  template <class ObjectT>
  Attribute(ObjectT* object, T (ObjectT::*method)()) {
    bind(object, method);
  }

  template <class ObjectT>
  Attribute(const ObjectT* object, T (ObjectT::*method)() const) {
    bind(object, method);
  }

  auto set(T value) -> void {
    m_value = std::move(value);
    m_callback.clear();
  }

  template <class Fn>
    requires std::is_invocable_r_v<T, Fn>
  auto bind(Fn callback) -> void {
    m_callback.bind(std::move(callback));
  }

  template <class ObjectT>
  auto bind(ObjectT* object, T (ObjectT::*method)()) -> void {
    m_callback.bind(object, method);
  }

  template <class ObjectT>
  auto bind(const ObjectT* object, T (ObjectT::*method)() const) -> void {
    m_callback.bind(object, method);
  }

  auto is_bound() const -> bool { return m_callback.is_bound(); }
  explicit operator bool() const { return is_bound(); }

  auto get() const -> T {
    if (m_callback) {
      return m_callback.execute();
    }
    return m_value;
  }

private:
  T m_value {};
  Delegate<T> m_callback;
};

} // namespace zod::sodium
