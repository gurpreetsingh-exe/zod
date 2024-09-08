namespace zod {

// template <typename T>
// concept Force = requires(T a) { a.update(); };

template <typename T, typename UpdateFn>
class Lazy {
public:
  Lazy(T inner) : m_inner(std::move(inner)) {}

public:
  auto operator->() -> T& {
    if (m_poisoned) {
      m_update(m_inner);
    }
    return m_inner;
  }

private:
  UpdateFn m_update;
  T m_inner;
  bool m_poisoned = false;
};

} // namespace zod
