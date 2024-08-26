#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace zod {

struct alignas(16) Node {
  glm::vec2 location = { 0, 0 };
  u32 id;
  u32 extra = 0;

  Node(u32 Id) : id(Id) {}
  Node(u32 Id, glm::vec2 Loc) : id(Id), location(Loc) {}
};

static_assert(sizeof(Node) == 16);

class NodeTree {
public:
  NodeTree() = default;

public:
  template <typename... Args>
  auto add_node(Args... args) -> Node& {
    auto& node = m_nodes.emplace_back(m_nodes.size() + 1, args...);
    return node;
  }
  auto get_data() const -> const Node* { return m_nodes.data(); }
  auto get_size() const -> const usize { return m_nodes.size() * sizeof(Node); }

private:
  std::vector<Node> m_nodes;
};

} // namespace zod
