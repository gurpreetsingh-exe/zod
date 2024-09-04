#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "nodes/node_tree.hh"

namespace zod {

NodeInitFn node_init_functions[TOTAL_NODES] = {
  [NODE_NONE] = [](Node& node) {},
  [NODE_FILE] =
      [](Node& node) { node.props.push_back(Property("Path", new char[64])); },
};

NodeDrawFn node_draw_functions[TOTAL_NODES] = {
  [NODE_NONE] = [](Node& node) {},
  [NODE_FILE] =
      [](Node& node) {
        for (auto& prop : node.props) { prop.draw(); }
      },
};

NodeUpdateFn node_update_functions[TOTAL_NODES];

const char* node_names[TOTAL_NODES] = {
  [NODE_NONE] = "None",
  [NODE_FILE] = "File",
};

} // namespace zod
