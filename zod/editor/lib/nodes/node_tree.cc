#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "application/context.hh"
#include "nodes/node_tree.hh"

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

        node.props.push_back(
            Property("Path", new char[64], PROP_SUBTYPE_FILEPATH));
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
        for (auto& prop : node.props) { prop.draw(); }
      },
};

NodeUpdateFn node_update_functions[TOTAL_NODES] = {
  NODE_NONE_PLACEHOLDER,
  [NODE_FILE] =
      [](Node& node) {
        const auto& path = GET_PROP(NODE_FILE_PATH);
        if (strnlen(path.s, 64) == 0) {
          return;
        }
        auto& mgr = ZCtxt::get().get_asset_manager();
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

auto NodeTree::add_link_partial(Node* from) -> NodeLink* {
  m_links.push_back(NodeLink());
  auto* link = &m_links[m_links.size() - 1];
  link->node_from = from;
  link->socket_from = &from->outputs.back();
  from->outputs[0].links.push_back(link);
  return link;
}

auto NodeTree::connect_link(NodeLink* link, Node* to) -> void {
  link->node_to = to;
  link->socket_to = &to->inputs.back();
  to->inputs[0].links.push_back(link);
}

auto NodeTree::add_link(Node* from, Node* to) -> NodeLink* {
  auto* link = add_link_partial(from);
  connect_link(link, to);
  return link;
}

auto NodeTree::remove_link(NodeLink* link) -> void {
  auto offset = link - m_links.data();
  auto it = m_links.begin() + offset;
  m_links.erase(it, it + 1);
}

} // namespace zod
