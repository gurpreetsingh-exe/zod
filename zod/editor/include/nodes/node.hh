#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "property.hh"

namespace zod {

enum {
  NODE_NONE = 0,
  NODE_FILE,

  TOTAL_NODES,
};

struct Node;

using NodeInitFn = std::function<void(Node&)>;
using NodeDrawFn = std::function<void(Node&)>;
using NodeUpdateFn = std::function<void(Node&)>;

extern NodeInitFn node_init_functions[TOTAL_NODES];
extern NodeDrawFn node_draw_functions[TOTAL_NODES];
extern NodeUpdateFn node_update_functions[TOTAL_NODES];

struct alignas(16) NodeType {
  glm::vec2 location = { 0, 0 };
  u32 id;

  u32 selected : 1;
  u32 type : 9;
  u32 _padding : 22;

  NodeType(u32 Id) : id(Id) {}
  NodeType(u32 Id, glm::vec2 Loc) : id(Id), location(Loc) {}
};

struct Node {
  NodeType* type;
  std::vector<Property> props = {};
  NodeDrawFn draw;
  NodeUpdateFn update;
};

} // namespace zod
