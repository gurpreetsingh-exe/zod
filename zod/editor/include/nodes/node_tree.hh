#pragma once

#include "node.hh"

namespace zod {

constexpr usize MAX_NODES = 1024;

struct Mesh;

class NodeTree {
public:
  NodeTree() {
    m_type_infos.reserve(MAX_NODES);
    m_nodes.reserve(MAX_NODES);
    auto format = std::vector<GPUBufferLayout> {
      { GPUDataType::Float, nullptr, 3, 0 },
    };
    m_batch = GPUBackend::get().create_batch(format);
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
    return node;
  }

  template <usize type, typename... Args>
  auto add_node(Args... args) -> Node& {
    return add_node(type, args...);
  }

  auto node_from_id(usize id) -> Node* {
    ZASSERT(id > 0);
    auto idx = id - 1;
    return &m_nodes.data()[idx];
  }

  template <typename UpdateFn>
  auto update_node(usize id, UpdateFn fn) -> void {
    fn(node_from_id(id));
  }

  auto update_batch(Mesh*) -> void;

  auto get_data() -> NodeType* { return m_type_infos.data(); }
  auto get_size() const -> const usize { return m_type_infos.size(); }
  auto get_nodes() -> std::vector<Node>& { return m_nodes; }
  auto get_visualized() -> u32 { return m_visualized; }
  auto get_batch() -> Shared<GPUBatch> { return m_batch; }
  auto set_visualized(u32 v) -> void { m_visualized = v; }

private:
  template <typename... Args>
  auto add_node(Args... args) -> Node& {
    auto id = m_type_infos.size() + 1;
    m_type_infos.push_back(NodeType(id, args...));
    auto* info = &m_type_infos.data()[id - 1];
    auto& node = m_nodes.emplace_back();
    node.type = info;
    return node;
  }

private:
  std::vector<NodeType> m_type_infos = {};
  std::vector<Node> m_nodes = {};
  Shared<GPUBatch> m_batch;
  u32 m_visualized = 0;
};

} // namespace zod
