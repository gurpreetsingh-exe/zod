#pragma once

#include "engine/property.hh"

namespace zod {

struct alignas(16) NodeType {
  vec2 location = { 0, 0 };
  u32 id;

  u32 selected : 1;
  u32 type : 9;
  u32 _padding : 22;

  NodeType(u32 Id) : id(Id) {}
  NodeType(u32 Id, vec2 Loc) : id(Id), location(Loc) {}
};

struct Node;
struct NodeSocket;
struct NodeLink;
class NodeTree;

using NodeInitFn = std::function<void(Node&)>;
using NodeDrawFn = std::function<void(Node&)>;
using NodeUpdateFn = std::function<void(Node&)>;

extern NodeInitFn node_init_functions[];
extern NodeDrawFn node_draw_functions[];
extern NodeUpdateFn node_update_functions[];
extern const char* node_names[];

struct Node {
  NodeType* type;
  NodeTree* parent;
  Vector<Property> props = {};
  Vector<NodeSocket> inputs = {};
  Vector<NodeSocket> outputs = {};
  NodeDrawFn draw;
  NodeUpdateFn update;
};

struct NodeSocket {
  u32 id;
  Vector<NodeLink*> links = {};
  NodeSocket() : id(0) {}
};

struct NodeLink {
  Node* node_from = nullptr;
  Node* node_to = nullptr;
  NodeSocket* socket_from = nullptr;
  NodeSocket* socket_to = nullptr;
};

/// Helper struct to extract extra flags from NodeType::id
struct NodeExtraFlags {
  union {
    struct {
      u8 visualize : 1;
      u8 socket_in : 1;
      u8 socket_out : 1;
      u8 padding : 5;
    };
    u8 data;
  };
  NodeExtraFlags(u8 d) : data(d) {}
};

static_assert(sizeof(NodeExtraFlags) == 1);

/// NodeTree ///////////////////////////////////////////////////////
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
  auto get_nodes() -> Vector<Node>& { return m_nodes; }
  auto get_links() -> Vector<NodeLink>& { return m_links; }
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
  Vector<NodeType> m_type_infos = {};
  Vector<Node> m_nodes = {};
  Vector<NodeLink> m_links = {};
  u32 m_visualized = 0;
  u32 m_active = 0;
};

} // namespace zod
