#pragma once

#include "application/event.hh"

namespace zod::sodium {

class EventHandler {
public:
  EventHandler() = default;

  template <class Fn>
  EventHandler(Fn callback) : m_callback(shared<Callback<Fn>>(callback)) {}

  template <class ObjectT>
  EventHandler(ObjectT* object,
               EventResponse (ObjectT::*method)(const Event&)) {
    bind(object, method);
  }

  template <class ObjectT>
  EventHandler(const ObjectT* object,
               EventResponse (ObjectT::*method)(const Event&) const) {
    bind(object, method);
  }

  template <class Fn>
  auto bind(Fn callback) -> void {
    m_callback = shared<Callback<Fn>>(callback);
  }

  template <class ObjectT>
  auto bind(ObjectT* object, EventResponse (ObjectT::*method)(const Event&))
      -> void {
    m_callback = shared<MethodCallback<ObjectT>>(object, method);
  }

  template <class ObjectT>
  auto bind(const ObjectT* object,
            EventResponse (ObjectT::*method)(const Event&) const) -> void {
    m_callback = shared<ConstMethodCallback<ObjectT>>(object, method);
  }

  auto is_bound() const -> bool { return bool(m_callback); }
  explicit operator bool() const { return is_bound(); }

  auto execute(const Event& event) const -> EventResponse {
    if (m_callback) {
      return m_callback->execute(event);
    }
    return EventResponse::unhandled();
  }

private:
  class CallbackBase {
  public:
    virtual ~CallbackBase() = default;
    virtual auto execute(const Event&) const -> EventResponse = 0;
  };

  template <class Fn>
  class Callback : public CallbackBase {
  public:
    explicit Callback(Fn callback) : m_callback(callback) {}

    auto execute(const Event& event) const -> EventResponse override {
      return m_callback(event);
    }

  private:
    Fn m_callback;
  };

  template <class ObjectT>
  class MethodCallback : public CallbackBase {
  public:
    MethodCallback(ObjectT* object,
                   EventResponse (ObjectT::*method)(const Event&))
        : m_object(object), m_method(method) {}

    auto execute(const Event& event) const -> EventResponse override {
      return (m_object->*m_method)(event);
    }

  private:
    ObjectT* m_object = nullptr;
    EventResponse (ObjectT::*m_method)(const Event&) = nullptr;
  };

  template <class ObjectT>
  class ConstMethodCallback : public CallbackBase {
  public:
    ConstMethodCallback(const ObjectT* object,
                        EventResponse (ObjectT::*method)(const Event&) const)
        : m_object(object), m_method(method) {}

    auto execute(const Event& event) const -> EventResponse override {
      return (m_object->*m_method)(event);
    }

  private:
    const ObjectT* m_object = nullptr;
    EventResponse (ObjectT::*m_method)(const Event&) const = nullptr;
  };

  SharedPtr<CallbackBase> m_callback = nullptr;
};

} // namespace zod::sodium
