#include <tiny_obj_loader.h>

#include "core/io/obj.hh"
#include "core/mesh/mesh.hh"

namespace zod {

auto load_obj(const fs::path& filepath) -> Mesh* {
  auto me = new Mesh();
  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig config;
  reader.ParseFromFile(filepath, config);

  if (not reader.Valid()) {
    fmt::println("{}", reader.Warning() + reader.Error());
    // eprintln("{}", reader.Warning() + reader.Error());
    // throw std::runtime_error(reader.Warning() + reader.Error());
  }

  std::unordered_map<Point, u32> unique_vertices;
  const auto& attributes = reader.GetAttrib();
  const auto& v = attributes.vertices;
  const auto& n = attributes.normals;

  for (const auto& shape : reader.GetShapes()) {
    const auto& mesh = shape.mesh;
    usize i = 0;
    for (auto count : mesh.num_face_vertices) {
      auto prim = Prim();
      for (usize idx = i; idx < i + count; ++idx) {
        auto index = mesh.indices[idx];
        auto point = Point();
        point.P = {
          v[3 * index.vertex_index + 0],
          v[3 * index.vertex_index + 1],
          v[3 * index.vertex_index + 2],
        };
        if (unique_vertices.count(point) == 0) {
          unique_vertices[point] = u32(me->points.size());
          me->points.push_back(point);
        }
        prim.points.push_back(unique_vertices[point]);
      }
      auto index_ = mesh.indices[i];
      vec3 normal = {
        n[3 * index_.normal_index + 0],
        n[3 * index_.normal_index + 1],
        n[3 * index_.normal_index + 2],
      };
      me->normals.push_back(normal);
      me->prims.push_back(std::move(prim));
      i += count;
    }
  }

  return me;
}

} // namespace zod
