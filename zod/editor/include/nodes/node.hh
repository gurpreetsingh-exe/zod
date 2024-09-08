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

struct Node {
  NodeType* type;
  std::vector<Property> props = {};
  NodeDrawFn draw;
  NodeUpdateFn update;
};

} // namespace zod
