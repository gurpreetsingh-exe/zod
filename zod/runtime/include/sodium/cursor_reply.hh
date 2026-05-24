#pragma once

#include "application/window.hh"

namespace zod::sodium {

class CursorReply {
public:
  static auto unhandled() -> CursorReply { return {}; }
  static auto cursor(cursor_shape_t shape) -> CursorReply {
    auto reply = CursorReply {};
    reply.m_handled = true;
    reply.m_shape = shape;
    return reply;
  }

  explicit operator bool() const { return m_handled; }
  auto shape() const -> cursor_shape_t { return m_shape; }

private:
  bool m_handled = false;
  cursor_shape_t m_shape = cursor_shape_t::Arrow;
};

} // namespace zod::sodium
