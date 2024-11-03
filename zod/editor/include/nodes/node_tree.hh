#pragma once

#include "node.hh"

namespace zod {

constexpr usize MAX_NODES = 1024;

class NodeTree {
public:
  NodeTree() {
    m_type_infos.reserve(MAX_NODES);
    m_nodes.reserve(MAX_NODES);
    m_links.reserve(MAX_NODES);
  }

public:
  template <typename... Args>
  auto add_node(usize type, Args... args) -> Node& {
    auto& node = add_node(args...);
    node.type->type = type;
    node.parent = this;
    node_init_functions[type](node);
    node.draw = node_draw_functions[type];
    node.update = node_update_functions[type];
    m_active = node.type->id;
    return node;
  }

  template <usize type, typename... Args>
  auto add_node(Args... args) -> Node& {
    return add_node(type, args...);
  }

  auto add_link_partial(Node* /* from */) -> NodeLink*;
  auto connect_link(NodeLink*, Node* /* to */) -> void;
  auto add_link(Node* /* from */, Node* /* to */) -> NodeLink*;
  auto remove_link(NodeLink*) -> void;

  auto node_from_id(usize id) -> Node* {
    ZASSERT(id > 0);
    auto idx = id - 1;
    return &m_nodes.data()[idx];
  }

  template <typename UpdateFn>
  auto update_node(usize id, UpdateFn fn) -> void {
    fn(node_from_id(id));
  }

  auto get_data() -> NodeType* { return m_type_infos.data(); }
  auto get_size() const -> const usize { return m_type_infos.size(); }
  auto get_nodes() -> std::vector<Node>& { return m_nodes; }
  auto get_links() -> std::vector<NodeLink>& { return m_links; }
  auto get_visualized() -> u32 { return m_visualized; }
  auto set_visualized(u32 v) -> void { m_visualized = v; }
  auto get_active() -> Node* {
    return m_active > 0 ? node_from_id(m_active) : nullptr;
  }
  auto set_active_id(u32 v) -> void { m_active = v; }
  auto get_active_id() -> u32 { return m_active; }

private:
  template <typename... Args>
  auto add_node(Args... args) -> Node& {
    auto id = m_type_infos.size() + 1;
    m_type_infos.push_back(NodeType(id, args...));
    auto* info = &m_type_infos.data()[id - 1];
    auto& node = m_nodes.emplace_back();
    node.type = info;
    m_active = node.type->id;
    return node;
  }

private:
  std::vector<NodeType> m_type_infos = {};
  std::vector<Node> m_nodes = {};
  std::vector<NodeLink> m_links = {};
  u32 m_visualized = 0;
  u32 m_active = 0;
};

} // namespace zod
