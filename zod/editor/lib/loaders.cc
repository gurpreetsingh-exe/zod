#include <tiny_gltf.h>
#include <tiny_obj_loader.h>
#include <yaml-cpp/yaml.h>

#include "core/uuid.hh"
#include "engine/mesh.hh"
#include "loaders.hh"

#include "gpu/backend.hh"

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

struct ParseCtxt {
  std::unordered_map<String, std::tuple<UUID, usize>> img_map;
  std::unordered_map<String, Material> materials;
};

auto process_nodes(const fs::path& save_dir, const tinygltf::Model& model,
                   const tinygltf::Node& node, ParseCtxt& pcx) -> void {
  const auto& me = model.meshes[node.mesh];
  auto points = Vector<Point>();
  auto normals = Vector<vec3>();
  auto uvs = Vector<vec2>();
  auto prims = Vector<u32>();
  auto submeshes = Vector<SubMesh>();

  for (auto primitive : me.primitives) {
    auto& submesh = submeshes.emplace_back();
    // index buffer
    {
      auto idx_accessor = model.accessors[primitive.indices];
      const auto& bufferView = model.bufferViews[idx_accessor.bufferView];
      const auto& buffer = model.buffers[bufferView.buffer];
      const auto* bytes =
          &buffer.data[bufferView.byteOffset + idx_accessor.byteOffset];
      u32 size = points.size();
      switch (idx_accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
          submesh.offset = prims.size();
          submesh.size = idx_accessor.count;
          auto indices = Span((u8*)bytes, idx_accessor.count);
          for (int i = 0; i < indices.size(); i++) {
            prims.push_back(indices[i] + size);
          }
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
          submesh.offset = prims.size();
          submesh.size = idx_accessor.count;
          auto indices = Span((u16*)bytes, idx_accessor.count);
          for (int i = 0; i < indices.size(); i++) {
            prims.push_back(indices[i] + size);
          }
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
          submesh.offset = prims.size();
          submesh.size = idx_accessor.count;
          auto indices = Span((u32*)bytes, idx_accessor.count);
          for (int i = 0; i < indices.size(); i++) {
            prims.push_back(indices[i] + size);
          }
        } break;
      }
    }

    for (const auto& attrib : primitive.attributes) {
      auto accessor = model.accessors[attrib.second];
      const auto& bufferView = model.bufferViews[accessor.bufferView];
      const auto& buffer = model.buffers[bufferView.buffer];
      const auto* bytes =
          &buffer.data[bufferView.byteOffset + accessor.byteOffset];
      if (attrib.first == "POSITION") {
        auto* buf = (Point*)bytes;
        points.insert(points.end(), buf, buf + accessor.count);
      } else if (attrib.first == "NORMAL") {
        auto* buf = (vec3*)bytes;
        normals.insert(normals.end(), buf, buf + accessor.count);
      } else if (attrib.first == "TEXCOORD_0") {
        auto* buf = (vec2*)bytes;
        uvs.insert(uvs.end(), buf, buf + accessor.count);
      }
    }

    if (primitive.material < 0) {
      continue;
    }

    auto& mat = model.materials[primitive.material];
    submesh.mat = pcx.materials[mat.name];
  }

  // remove duplicate points
  // auto pts = Vector<Point>();
  // auto nrm = Vector<vec3>();
  // auto buf = (Prim*)prims.data();
  // auto unique_vertices = std::unordered_map<Point, u32>();
  // auto points_i = usize(0);
  // for (int i = 0; i < prims.size() / 3; ++i) {
  //   auto& prim = buf[i];
  //   auto& pti = prim.points;
  //   for (int j = 0; j < 3; ++j) {
  //     auto idx = pti[j];
  //     auto P = points[idx];
  //     auto N = normals[idx];
  //     if (not unique_vertices.contains(P)) {
  //       unique_vertices[P] = u32(pts.size());
  //       pts.push_back(P);
  //       nrm.push_back(N);
  //     }
  //     pti[j] = unique_vertices[P];
  //   }
  // }

  auto mesh = Mesh();
  mesh.points = Span(points.data(), points.size());
  mesh.normals = Span(normals.data(), normals.size());
  mesh.uvs = Span(uvs.data(), uvs.size());
  mesh.prims = Span((Prim*)prims.data(), prims.size() / 3);
  mesh.submeshes = Span(submeshes.data(), submeshes.size());
  auto ar = Archive();
  mesh.write(ar);
  auto name = me.name.empty() ? "0" : me.name;
  ar.save(save_dir / "Meshes" / name, ".zmesh");

  for (const auto& idx : node.children) {
    process_nodes(save_dir, model, model.nodes[idx], pcx);
  }
}

auto loadGLTF(const fs::path& save_dir, const fs::path& path) -> void {
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
  auto pcx = ParseCtxt();

  /// TEXTURES
  {
    auto texture_idx = 0ZU;
    auto out = YAML::Emitter();
    out << YAML::BeginMap;
    out << YAML::Key << "Textures" << YAML::Value << YAML::BeginSeq;
    for (const auto& img : model.images) {
      const auto& buf = img.image;
      auto ar = Archive();
      auto size = buf.size();
      usize w = img.width;
      usize h = img.height;
      ar.copy((u8*)&w, sizeof(usize));
      ar.copy((u8*)&h, sizeof(usize));
      ar.copy((u8*)&size, sizeof(usize));
      ar.copy((u8*)buf.data(), size);
      auto name = img.name.empty() ? img.uri.empty() ? "" : img.uri : img.name;
      ZASSERT(not name.empty());
      ar.save(save_dir / "Textures" / name, ".ztex");

      out << YAML::BeginMap;
      auto p = save_dir / "Textures" / name;
      p.replace_extension(".ztex");
      out << YAML::Key << "Path" << YAML::Value << p.string();
      auto id = UUID();
      out << YAML::Key << "UUID" << YAML::Value << id.to_string();
      pcx.img_map[name] = { id, texture_idx };
      out << YAML::EndMap;
      texture_idx += 1;
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;

    auto ar = Archive();
    ar.copy((u8*)out.c_str(), out.size());
    ar.save(save_dir / "Textures", ".meta");
  }

  /// MATERIALS
  {
    auto mat_idx = 0ZU;
    for (auto& mat : model.materials) {
      if (mat.name.empty()) {
        mat.name = fmt::format("Material{}", mat_idx);
      }

      const auto& pbr = mat.pbrMetallicRoughness;
      if (pbr.baseColorTexture.index >= 0) {
        auto find_texture_index = [&](int gltf_index) {
          if (gltf_index < 0) {
            return -1ZU;
          }
          auto& texture = model.textures[gltf_index];
          auto& img = model.images[texture.source];
          auto name =
              img.name.empty() ? img.uri.empty() ? "" : img.uri : img.name;
          ZASSERT(not name.empty());
          return std::get<1>(pcx.img_map[name]);
        };

        pcx.materials[mat.name] = Material {
          .color_texture = find_texture_index(pbr.baseColorTexture.index),
          .normal_texture = find_texture_index(mat.normalTexture.index),
        };
      }
      mat_idx += 1;
    }
  }

  for (auto idx : scene.nodes) {
    process_nodes(save_dir, model, model.nodes[idx], pcx);
  }
}

} // namespace zod
