#pragma once

namespace zod::sodium {

template <class ResultT, class... Args>
class Delegate {
public:
  Delegate() = default;

  template <class Fn>
    requires std::is_invocable_r_v<ResultT, Fn, Args...>
  Delegate(Fn callback) {
    bind(std::move(callback));
  }

  template <class ObjectT>
  Delegate(ObjectT* object, ResultT (ObjectT::*method)(Args...)) {
    bind(object, method);
  }

  template <class ObjectT>
  Delegate(const ObjectT* object, ResultT (ObjectT::*method)(Args...) const) {
    bind(object, method);
  }

  template <class Fn>
    requires std::is_invocable_r_v<ResultT, Fn, Args...>
  auto bind(Fn callback) -> void {
    m_callback = shared<Callback<Fn>>(std::move(callback));
  }

  template <class ObjectT>
  auto bind(ObjectT* object, ResultT (ObjectT::*method)(Args...)) -> void {
    m_callback = shared<MethodCallback<ObjectT>>(object, method);
  }

  template <class ObjectT>
  auto bind(const ObjectT* object, ResultT (ObjectT::*method)(Args...) const)
      -> void {
    m_callback = shared<ConstMethodCallback<ObjectT>>(object, method);
  }

  auto clear() -> void { m_callback = nullptr; }
  auto is_bound() const -> bool { return bool(m_callback); }
  explicit operator bool() const { return is_bound(); }

  auto execute(Args... args) const -> ResultT {
    return m_callback->execute(std::forward<Args>(args)...);
  }

private:
  class CallbackBase {
  public:
    virtual ~CallbackBase() = default;
    virtual auto execute(Args...) const -> ResultT = 0;
  };

  template <class Fn>
  class Callback : public CallbackBase {
  public:
    explicit Callback(Fn callback) : m_callback(std::move(callback)) {}

    auto execute(Args... args) const -> ResultT override {
      return m_callback(std::forward<Args>(args)...);
    }

  private:
    Fn m_callback;
  };

  template <class ObjectT>
  class MethodCallback : public CallbackBase {
  public:
    MethodCallback(ObjectT* object, ResultT (ObjectT::*method)(Args...))
        : m_object(object), m_method(method) {}

    auto execute(Args... args) const -> ResultT override {
      return (m_object->*m_method)(std::forward<Args>(args)...);
    }

  private:
    ObjectT* m_object = nullptr;
    ResultT (ObjectT::*m_method)(Args...) = nullptr;
  };

  template <class ObjectT>
  class ConstMethodCallback : public CallbackBase {
  public:
    ConstMethodCallback(const ObjectT* object,
                        ResultT (ObjectT::*method)(Args...) const)
        : m_object(object), m_method(method) {}

    auto execute(Args... args) const -> ResultT override {
      return (m_object->*m_method)(std::forward<Args>(args)...);
    }

  private:
    const ObjectT* m_object = nullptr;
    ResultT (ObjectT::*m_method)(Args...) const = nullptr;
  };

  SharedPtr<CallbackBase> m_callback = nullptr;
};

} // namespace zod::sodium
