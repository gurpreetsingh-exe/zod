#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "editor.hh"
#include "engine/node_types.hh"
#include "nodes.hh"

namespace zod {

#define NODE_NONE_PLACEHOLDER [NODE_NONE] = [](Node&) {}
#define GET_PROP(prop) node.props[prop]

/// File Node
enum {
  NODE_FILE_PATH,

  NODE_FILE_PROPS,
};

NodeInitFn node_init_functions[TOTAL_NODES] = {
  NODE_NONE_PLACEHOLDER,
  [NODE_FILE] =
      [](Node& node) {
        node.outputs.push_back(NodeSocket());

        node.props.push_back(Property("Path", new char[STRING_PROP_MAX_SIZE],
                                      PROP_SUBTYPE_FILEPATH));
      },
  [NODE_TRANSFORM] =
      [](Node& node) {
        node.inputs.push_back(NodeSocket());
        node.outputs.push_back(NodeSocket());

        node.props.push_back(Property("Location", vec3(0.0f)));
        node.props.push_back(Property("Rotation", vec3(0.0f)));
        node.props.push_back(Property("Scale", vec3(1.0f)));
      },
};

NodeDrawFn node_draw_functions[TOTAL_NODES] = {
  NODE_NONE_PLACEHOLDER,
  [NODE_FILE] =
      [](Node& node) {
        for (auto& prop : node.props) { draw_property(prop); }
      },
};

NodeUpdateFn node_update_functions[TOTAL_NODES] = {
  NODE_NONE_PLACEHOLDER,
  [NODE_FILE] =
      [](Node& node) {
        const auto& path = GET_PROP(NODE_FILE_PATH);
        if (strnlen(path.s, STRING_PROP_MAX_SIZE) == 0) {
          return;
        }
        auto& mgr = Editor::get().get_asset_manager();
        auto id = mgr.load(path.s);
        if (id == uuid0()) {
          return;
        }
        ZASSERT(node.parent);
      },
  [NODE_TRANSFORM] = [](Node& node) {},
};

const char* node_names[TOTAL_NODES] = {
  [NODE_NONE] = "None",
  [NODE_FILE] = "File",
  [NODE_TRANSFORM] = "Transform",
};

} // namespace zod
