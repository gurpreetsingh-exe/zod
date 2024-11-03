#pragma once

#include "property.hh"

namespace zod {

enum {
  NODE_NONE = 0,
  NODE_FILE,
  NODE_TRANSFORM,

  TOTAL_NODES,
};

constexpr f32 NODE_SIZE = 100.0f;

struct Node;

using NodeInitFn = std::function<void(Node&)>;
using NodeDrawFn = std::function<void(Node&)>;
using NodeUpdateFn = std::function<void(Node&)>;

extern NodeInitFn node_init_functions[TOTAL_NODES];
extern NodeDrawFn node_draw_functions[TOTAL_NODES];
extern NodeUpdateFn node_update_functions[TOTAL_NODES];
extern const char* node_names[TOTAL_NODES];

struct alignas(16) NodeType {
  vec2 location = { 0, 0 };
  u32 id;

  u32 selected : 1;
  u32 type : 9;
  u32 _padding : 22;

  NodeType(u32 Id) : id(Id) {}
  NodeType(u32 Id, vec2 Loc) : id(Id), location(Loc) {}
};

class NodeTree;
struct NodeSocket;
struct NodeLink;

struct Node {
  NodeType* type;
  NodeTree* parent;
  std::vector<Property> props = {};
  std::vector<NodeSocket> inputs = {};
  std::vector<NodeSocket> outputs = {};
  NodeDrawFn draw;
  NodeUpdateFn update;
};

struct NodeSocket {
  u32 id;
  std::vector<NodeLink*> links = {};
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

} // namespace zod
