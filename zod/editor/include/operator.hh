#pragma once

#include "event.hh"

namespace zod {

class Panel;

enum OperatorState : u8 {
  OP_RUNNING,
  OP_FINISHED,
  OP_CANCELLED,
};

class Operator {
protected:
  Operator(Panel* panel) : m_panel(panel) {}

public:
  virtual ~Operator() = default;
  virtual auto execute(Event&) -> u8 = 0;
  auto set_data_ptr(void* data) -> void { m_data = data; }

protected:
  Panel* m_panel;
  void* m_data = nullptr;
};

} // namespace zod
