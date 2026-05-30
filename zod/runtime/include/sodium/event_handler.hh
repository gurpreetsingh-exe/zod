#pragma once

#include "application/event.hh"
#include "sodium/delegate.hh"

namespace zod::sodium {

class EventHandler {
public:
  EventHandler() = default;

  template <class Fn>
    requires std::is_invocable_r_v<EventResponse, Fn, const Event&>
  EventHandler(Fn callback) : m_callback(std::move(callback)) {}

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
    requires std::is_invocable_r_v<EventResponse, Fn, const Event&>
  auto bind(Fn callback) -> void {
    m_callback.bind(std::move(callback));
  }

  template <class ObjectT>
  auto bind(ObjectT* object, EventResponse (ObjectT::*method)(const Event&))
      -> void {
    m_callback.bind(object, method);
  }

  template <class ObjectT>
  auto bind(const ObjectT* object,
            EventResponse (ObjectT::*method)(const Event&) const) -> void {
    m_callback.bind(object, method);
  }

  auto is_bound() const -> bool { return m_callback.is_bound(); }
  explicit operator bool() const { return is_bound(); }

  auto execute(const Event& event) const -> EventResponse {
    if (m_callback) {
      return m_callback.execute(event);
    }
    return EventResponse::unhandled();
  }

private:
  Delegate<EventResponse, const Event&> m_callback;
};

} // namespace zod::sodium
