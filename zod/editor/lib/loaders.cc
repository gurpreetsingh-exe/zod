#include <tiny_gltf.h>
#include <tiny_obj_loader.h>

#include "engine/mesh.hh"
#include "loaders.hh"

namespace zod {

auto load_obj(const fs::path& filepath) -> SharedPtr<Mesh> {
  auto me = shared<Mesh>();
  // tinyobj::ObjReader reader;
  // tinyobj::ObjReaderConfig config;
  // reader.ParseFromFile(filepath, config);

  // if (not reader.Valid()) {
  //   fmt::println("{}", reader.Warning() + reader.Error());
  //   // eprintln("{}", reader.Warning() + reader.Error());
  //   // throw std::runtime_error(reader.Warning() + reader.Error());
  // }

  // std::unordered_map<Point, u32> unique_vertices;
  // const auto& attributes = reader.GetAttrib();
  // const auto& v = attributes.vertices;
  // const auto& n = attributes.normals;

  // for (const auto& shape : reader.GetShapes()) {
  //   const auto& mesh = shape.mesh;
  //   usize i = 0;
  //   for (auto count : mesh.num_face_vertices) {
  //     ZASSERT(count == 3, "unexpected vertex count in faces");
  //     auto prim = Prim();
  //     usize vertex_index = 0;
  //     for (usize idx = i; idx < i + count; ++idx) {
  //       auto index = mesh.indices[idx];
  //       auto point = Point();
  //       point.P = {
  //         v[3 * index.vertex_index + 0],
  //         v[3 * index.vertex_index + 1],
  //         v[3 * index.vertex_index + 2],
  //       };
  //       if (unique_vertices.count(point) == 0) {
  //         unique_vertices[point] = u32(me->points.size());
  //         me->points.push_back(point);
  //       }
  //       prim.points[vertex_index++] = unique_vertices[point];
  //     }
  //     auto index_ = mesh.indices[i];
  //     vec3 normal = {
  //       n[3 * index_.normal_index + 0],
  //       n[3 * index_.normal_index + 1],
  //       n[3 * index_.normal_index + 2],
  //     };
  //     me->normals.push_back(normal);
  //     me->prims.push_back(prim);
  //     i += count;
  //   }
  // }

  return me;
}

auto process_nodes(const tinygltf::Model& model, const tinygltf::Node& node)
    -> void {
  const auto& me = model.meshes[node.mesh];
  for (auto primitive : me.primitives) {
    auto mesh = Mesh();
    // index buffer
    auto prims = Vector<Prim>();
    auto face_normals = Vector<vec3>();
    {
      auto idx_accessor = model.accessors[primitive.indices];
      const auto& bufferView = model.bufferViews[idx_accessor.bufferView];
      const auto& buffer = model.buffers[bufferView.buffer];
      const auto* bytes = &buffer.data.data()[bufferView.byteOffset];
      switch (idx_accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
          auto indices = Span((u8*)bytes, bufferView.byteLength / sizeof(u8));
          for (int i = 0; i < indices.size(); i += 3) {
            prims.push_back(
                { .points = { indices[i], indices[i + 1], indices[i + 2] } });
          }
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
          auto indices = Span((u16*)bytes, bufferView.byteLength / sizeof(u16));
          for (int i = 0; i < indices.size(); i += 3) {
            prims.push_back(
                { .points = { indices[i], indices[i + 1], indices[i + 2] } });
          }
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
          prims = Vector<Prim>((Prim*)bytes,
                               (Prim*)bytes +
                                   (bufferView.byteLength / sizeof(Prim)));
        } break;
      }
      mesh.prims = Span((Prim*)prims.data(), prims.size());
    }

    for (const auto& attrib : primitive.attributes) {
      auto accessor = model.accessors[attrib.second];
      const auto& bufferView = model.bufferViews[accessor.bufferView];
      const auto& buffer = model.buffers[bufferView.buffer];
      const auto* bytes = &buffer.data.data()[bufferView.byteOffset];
      if (attrib.first == "POSITION") {
        mesh.points =
            Span((Point*)bytes, bufferView.byteLength / sizeof(Point));
      } else if (attrib.first == "NORMAL") {
        auto normals = Span((vec3*)bytes, bufferView.byteLength / sizeof(vec3));
        for (auto prim : mesh.prims) {
          auto [i1, i2, i3] = prim.points;
          auto n1 = normals[i1];
          auto n2 = normals[i2];
          auto n3 = normals[i3];
          ZASSERT(n1 == n2 and n2 == n3);
          face_normals.push_back(n1);
        }
        mesh.normals = Span((vec3*)face_normals.data(), face_normals.size());
      }
    }
    auto ar = Archive();
    mesh.write(ar);
    ar.save(fmt::format("{}.zmesh", me.name));
  }

  for (const auto& idx : node.children) {
    process_nodes(model, model.nodes[idx]);
  }
}

auto loadGLTF(const fs::path& path) -> SharedPtr<Mesh> {
  auto mesh = shared<Mesh>();
  tinygltf::Model model;
  tinygltf::TinyGLTF gcx;
  String err;
  String warn;

  gcx.SetStoreOriginalJSONForExtrasAndExtensions(true);

  auto ret = false;
  ret = path.extension() == ".glb"
            ? gcx.LoadBinaryFromFile(&model, &err, &warn, path.string().c_str())
            : gcx.LoadASCIIFromFile(&model, &err, &warn, path.string().c_str());

  if (not warn.empty()) {
    eprintln("Warning: {}", warn);
  }

  if (not err.empty()) {
    eprintln("Error: {}", err);
  }

  if (not ret) {
    eprintln("Failed to parse glTF");
  }

  const auto& scene = model.scenes[model.defaultScene];
  auto transform = mat4(1.0f);

  for (auto idx : scene.nodes) { process_nodes(model, model.nodes[idx]); }

  return mesh;
}

} // namespace zod
