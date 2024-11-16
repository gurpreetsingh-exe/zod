#pragma once

#include "operator.hh"

namespace zod {

struct Node;
struct NodeLink;

// TODO: create the operator with an offset input
class OpNodeTransform : public Operator {
public:
  OpNodeTransform(SPanel* panel) : Operator(panel) {}

public:
  auto execute(Event& event) -> u8 override;
};

class OpNodeLinkCreateNew : public Operator {
public:
  OpNodeLinkCreateNew(SPanel* panel, Node* from)
      : Operator(panel), m_from(from) {}

public:
  auto execute(Event& event) -> u8 override;

private:
  Node* m_from = nullptr;
  NodeLink* m_link = nullptr;
};

class OpNodeAdd : public Operator {
public:
  OpNodeAdd(SPanel* panel, Node* from) : Operator(panel) {}

public:
  auto execute(Event& event) -> u8 override;
};

} // namespace zod
