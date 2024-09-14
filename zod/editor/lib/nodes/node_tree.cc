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
        node.props.push_back(
            Property("Path", new char[64], PROP_SUBTYPE_FILEPATH));
      },
  [NODE_TRANSFORM] =
      [](Node& node) {
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
        node.parent->update_batch(mgr.get(id));
      },
  [NODE_TRANSFORM] = [](Node& node) {},
};

const char* node_names[TOTAL_NODES] = {
  [NODE_NONE] = "None",
  [NODE_FILE] = "File",
  [NODE_TRANSFORM] = "Transform",
};

auto NodeTree::update_batch(Mesh* mesh) -> void {
  auto format = std::vector<GPUBufferLayout> {
    { GPUDataType::Float, mesh->points.data(), 3, mesh->points.size() * 3 },
  };
  auto indices = std::vector<u32>();
  for (const auto& prim : mesh->prims) {
    for (auto i : prim.points) { indices.push_back(i); }
  }

  m_batch = GPUBackend::get().create_batch(format, indices);
  ZCtxt::get().get_normals()->upload_data(mesh->normals.data(),
                                          mesh->normals.size() * sizeof(vec3));
}

} // namespace zod
