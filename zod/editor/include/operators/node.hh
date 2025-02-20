#pragma once

#include "core/operator.hh"

namespace zod {

struct Node;
struct NodeLink;

// TODO: create the operator with an offset input
class OpNodeTransform : public IOperator {
public:
  OpNodeTransform(SPanel* panel) : IOperator(panel) {}

public:
  auto execute(Event& event) -> u8 override;
};

class OpNodeLinkCreateNew : public IOperator {
public:
  OpNodeLinkCreateNew(SPanel* panel, Node* from)
      : IOperator(panel), m_from(from) {}

public:
  auto execute(Event& event) -> u8 override;

private:
  Node* m_from = nullptr;
  NodeLink* m_link = nullptr;
};

class OpNodeAdd : public IOperator {
public:
  OpNodeAdd(SPanel* panel, Node* from) : IOperator(panel) {}

public:
  auto execute(Event& event) -> u8 override;
};

} // namespace zod
