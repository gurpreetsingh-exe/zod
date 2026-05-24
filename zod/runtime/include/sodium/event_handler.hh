#pragma once

#include "application/event.hh"

namespace zod::sodium {

class EventHandler {
public:
  EventHandler() = default;

  template <class Fn>
  EventHandler(Fn callback) : m_callback(shared<Callback<Fn>>(callback)) {}

  template <class Fn>
  auto bind(Fn callback) -> void {
    m_callback = shared<Callback<Fn>>(callback);
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

  SharedPtr<CallbackBase> m_callback = nullptr;
};

} // namespace zod::sodium
