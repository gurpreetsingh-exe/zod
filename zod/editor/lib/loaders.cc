#include <tiny_gltf.h>
#include <tiny_obj_loader.h>
#include <yaml-cpp/yaml.h>
#include <glm/gtx/matrix_decompose.hpp>

#include "core/uuid.hh"
#include "engine/asset_database.hh"
#include "engine/material.hh"
#include "engine/mesh.hh"
#include "engine/project.hh"
#include "engine/scene.hh"
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
  Vector<std::tuple<UUID, usize>> images;
  Vector<UUID> materials;
  Vector<fs::path> meshes;
};

static auto sanitize_filename(String name) -> String {
  std::replace(name.begin(), name.end(), '.', '_');
  std::replace(name.begin(), name.end(), '/', '_');
  std::replace(name.begin(), name.end(), '\\', '_');
  return name;
}

static auto mesh_path(const tinygltf::Model& model, int mesh_index)
    -> fs::path {
  const auto& me = model.meshes[mesh_index];
  auto name = me.name.empty() ? fmt::format("Mesh{}", mesh_index) : me.name;
  name = sanitize_filename(fmt::format("{}_{}", mesh_index, name));
  auto path = fs::path("Meshes") / name;
  path.replace_extension(".zmesh");
  return path;
}

static auto import_mesh(const fs::path& save_dir, const tinygltf::Model& model,
                        int mesh_index, ParseCtxt& pcx) -> fs::path {
  if (mesh_index < 0 or mesh_index >= model.meshes.size()) {
    return {};
  }

  if (pcx.meshes.empty()) {
    pcx.meshes.resize(model.meshes.size());
  }

  if (not pcx.meshes[mesh_index].empty()) {
    return pcx.meshes[mesh_index];
  }

  const auto& me = model.meshes[mesh_index];
  auto path = mesh_path(model, mesh_index);
  pcx.meshes[mesh_index] = path;

  {
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

      if (primitive.material < pcx.materials.size()) {
        submesh.mat = pcx.materials[primitive.material];
      }
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
    mesh.submeshes = submeshes;
    auto ar = Archive();
    mesh.write(ar);
    ar.save(save_dir / path);
  }

  return path;
}

static auto write_vec3(YAML::Emitter& out, const char* key, vec3 value)
    -> void {
  out << YAML::Key << key << YAML::Value << YAML::Flow << YAML::BeginSeq
      << value.x << value.y << value.z << YAML::EndSeq;
}

static auto write_transform(YAML::Emitter& out, vec3 position, vec3 rotation,
                            vec3 scale) -> void {
  out << YAML::Key << "TransformComponent";
  out << YAML::BeginMap;
  write_vec3(out, "Position", position);
  write_vec3(out, "Rotation", rotation);
  write_vec3(out, "Scale", scale);
  out << YAML::EndMap;
}

static auto write_scene_entity(YAML::Emitter& out, UUID id, const String& name)
    -> void {
  out << YAML::BeginMap;
  out << YAML::Key << "Entity" << YAML::Value << id.to_string();
  out << YAML::Key << "IdentifierComponent";
  out << YAML::BeginMap;
  out << YAML::Key << "Identifier" << YAML::Value << name;
  out << YAML::EndMap;
}

static auto node_local_matrix(const tinygltf::Node& node) -> mat4 {
  if (node.matrix.size() == 16) {
    auto matrix = mat4(1.0f);
    for (auto column = 0; column < 4; ++column) {
      for (auto row = 0; row < 4; ++row) {
        matrix[column][row] = f32(node.matrix[column * 4 + row]);
      }
    }
    return matrix;
  }

  auto t = vec3(0.0f);
  if (node.translation.size() == 3) {
    t = vec3(f32(node.translation[0]), f32(node.translation[1]),
             f32(node.translation[2]));
  }

  auto r = quat(1.0f, 0.0f, 0.0f, 0.0f);
  if (node.rotation.size() == 4) {
    r = quat(f32(node.rotation[3]), f32(node.rotation[0]),
             f32(node.rotation[1]), f32(node.rotation[2]));
  }

  auto s = vec3(1.0f);
  if (node.scale.size() == 3) {
    s = vec3(f32(node.scale[0]), f32(node.scale[1]), f32(node.scale[2]));
  }

  return translate(mat4(1.0f), t) * toMat4(r) * glm::scale(mat4(1.0f), s);
}

static auto decompose_transform(mat4 matrix, vec3& position, vec3& rotation,
                                vec3& scale) -> void {
  auto orientation = quat();
  auto skew = vec3();
  auto perspective = vec4();
  glm::decompose(matrix, scale, orientation, position, skew, perspective);
  rotation = glm::degrees(glm::eulerAngles(orientation));
}

static auto write_gltf_node_scene(const fs::path& save_dir,
                                  const tinygltf::Model& model, int node_index,
                                  mat4 parent, ParseCtxt& pcx,
                                  YAML::Emitter& out) -> void {
  if (node_index < 0 or node_index >= model.nodes.size()) {
    return;
  }

  const auto& node = model.nodes[node_index];
  auto world = parent * node_local_matrix(node);
  if (node.mesh >= 0 and node.mesh < model.meshes.size()) {
    auto mesh = import_mesh(save_dir, model, node.mesh, pcx);
    auto name = node.name.empty() ? model.meshes[node.mesh].name : node.name;
    if (name.empty()) {
      name = fmt::format("Node{}", node_index);
    }

    auto position = vec3();
    auto rotation = vec3();
    auto scale = vec3(1.0f);
    decompose_transform(world, position, rotation, scale);

    write_scene_entity(out, UUID(), name);
    write_transform(out, position, rotation, scale);
    out << YAML::Key << "StaticMeshComponent";
    out << YAML::BeginMap;
    out << YAML::Key << "Mesh" << YAML::Value << mesh.string();
    out << YAML::EndMap;
    out << YAML::EndMap;
  }

  for (auto child : node.children) {
    write_gltf_node_scene(save_dir, model, child, world, pcx, out);
  }
}

static auto write_gltf_scene(const fs::path& save_dir, const fs::path& source,
                             const tinygltf::Model& model,
                             const tinygltf::Scene& scene, ParseCtxt& pcx)
    -> fs::path {
  auto scene_name = sanitize_filename(source.stem().string());
  if (scene_name.empty()) {
    scene_name = "ImportedGLTF";
  }

  auto camera_id = UUID();
  auto out = YAML::Emitter();
  out << YAML::BeginMap;
  out << YAML::Key << "Name" << YAML::Value << scene_name;
  out << YAML::Key << "CameraEntity" << YAML::Value << camera_id.to_string();
  out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

  write_scene_entity(out, camera_id, "SceneCamera");
  write_transform(out, vec3(0.0f), vec3(0.0f), vec3(1.0f));
  out << YAML::Key << "CameraComponent";
  out << YAML::BeginMap;
  out << YAML::Key << "PerspectiveCamera" << YAML::Value << YAML::BeginMap;
  out << YAML::Key << "Fov" << YAML::Value << 90.0f;
  out << YAML::Key << "ClipNear" << YAML::Value << 0.01f;
  out << YAML::Key << "ClipFar" << YAML::Value << 100.0f;
  out << YAML::EndMap;
  out << YAML::EndMap;
  out << YAML::EndMap;

  for (auto node : scene.nodes) {
    write_gltf_node_scene(save_dir, model, node, mat4(1.0f), pcx, out);
  }

  out << YAML::EndSeq;
  out << YAML::EndMap;

  auto scene_path = fs::path("Scenes") / scene_name;
  scene_path.replace_extension(".zscene");
  auto ar = Archive();
  ar.copy((u8*)out.c_str(), out.size());
  ar.save(save_dir / scene_path);
  return save_dir / scene_path;
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

  const auto scene_index = model.defaultScene >= 0 ? model.defaultScene : 0;
  ZASSERT(scene_index < model.scenes.size(), "glTF has no scenes");
  const auto& scene = model.scenes[scene_index];
  auto pcx = ParseCtxt();

  /// TEXTURES
  {
    auto texture_idx = 0ZU;
    auto mega_texture_assets = Vector<UUID>();
    auto mega_texture = YAML::Emitter();
    mega_texture << YAML::BeginMap;
    mega_texture << YAML::Key << "Textures" << YAML::Value << YAML::BeginSeq;
    pcx.images.reserve(model.images.size());
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

      auto artifact = fs::path("Textures") / name;
      artifact.replace_extension(".ztex");
      auto meta_path = fs::path("Textures") / name;
      meta_path.replace_extension(".meta");
      auto source = img.uri.empty() ? path.string() : img.uri;
      auto id = UUID();
      mega_texture_assets.push_back(id);

      auto texture_meta = YAML::Emitter();
      texture_meta << YAML::BeginMap;
      texture_meta << YAML::Key << "UUID" << YAML::Value << id.to_string();
      texture_meta << YAML::Key << "Kind" << YAML::Value << "Texture";
      texture_meta << YAML::Key << "Source" << YAML::Value << source;
      texture_meta << YAML::Key << "Artifact" << YAML::Value
                   << artifact.string();
      texture_meta << YAML::EndMap;

      auto texture_meta_ar = Archive();
      texture_meta_ar.copy((u8*)texture_meta.c_str(), texture_meta.size());
      texture_meta_ar.save(save_dir / meta_path);

      mega_texture << YAML::BeginMap;
      mega_texture << YAML::Key << "UUID" << YAML::Value << id.to_string();
      mega_texture << YAML::Key << "Meta" << YAML::Value << meta_path.string();
      mega_texture << YAML::EndMap;

      g_project->asset_database().register_asset(id, AssetKind::Texture,
                                                 artifact, source, meta_path);
      pcx.images.push_back({ id, texture_idx });
      texture_idx += 1;
    }
    mega_texture << YAML::EndSeq;
    mega_texture << YAML::EndMap;

    auto mega_texture_ar = Archive();
    mega_texture_ar.copy((u8*)mega_texture.c_str(), mega_texture.size());
    mega_texture_ar.save(save_dir / "MegaTexture", ".meta");
    g_project->asset_database().set_mega_texture_assets(
        std::move(mega_texture_assets));
  }

  /// MATERIALS
  {
    auto mat_idx = 0ZU;
    for (auto& mat : model.materials) {
      if (mat.name.empty()) {
        mat.name = fmt::format("Material{}", mat_idx);
      }

      const auto& pbr = mat.pbrMetallicRoughness;
      auto find_texture = [&](int gltf_index) {
        if (gltf_index < 0 or gltf_index >= model.textures.size()) {
          return UUID::null();
        }

        auto& texture = model.textures[gltf_index];
        if (texture.source < 0 or texture.source >= pcx.images.size()) {
          return UUID::null();
        }

        return std::get<0>(pcx.images[texture.source]);
      };

      auto id = UUID();
      auto material = Material {
        .color_texture = find_texture(pbr.baseColorTexture.index),
        .normal_texture = find_texture(mat.normalTexture.index),
        .roughness_texture = find_texture(pbr.metallicRoughnessTexture.index),
      };
      auto material_file = fmt::format("{}_{}", mat_idx, mat.name);
      std::replace(material_file.begin(), material_file.end(), '.', '_');
      std::replace(material_file.begin(), material_file.end(), '/', '_');
      std::replace(material_file.begin(), material_file.end(), '\\', '_');
      auto meta_path = fs::path("Materials") / material_file;
      meta_path.replace_extension(".meta");

      auto material_meta = YAML::Emitter();
      material_meta << YAML::BeginMap;
      material_meta << YAML::Key << "UUID" << YAML::Value << id.to_string();
      material_meta << YAML::Key << "Kind" << YAML::Value << "Material";
      material_meta << YAML::Key << "Source" << YAML::Value << path.string();
      material_meta << YAML::Key << "BaseColorTexture" << YAML::Value
                    << material.color_texture.to_string();
      material_meta << YAML::Key << "NormalTexture" << YAML::Value
                    << material.normal_texture.to_string();
      material_meta << YAML::Key << "RoughnessTexture" << YAML::Value
                    << material.roughness_texture.to_string();
      material_meta << YAML::EndMap;

      auto material_meta_ar = Archive();
      material_meta_ar.copy((u8*)material_meta.c_str(), material_meta.size());
      material_meta_ar.save(save_dir / meta_path);

      g_project->asset_database().register_asset(
          id, AssetKind::Material, meta_path, path.string(), meta_path);
      g_project->asset_database().register_material(id, material);

      pcx.materials.push_back(id);
      mat_idx += 1;
    }
  }

  auto scene_path = write_gltf_scene(save_dir, path, model, scene, pcx);
  if (g_project) {
    g_project->scene_manager().load_from_file(scene_path);
  }
}

} // namespace zod
