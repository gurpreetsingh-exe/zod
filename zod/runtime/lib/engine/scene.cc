#include "engine/scene.hh"
#include "./mesh_batch.hh"
#include "core/platform/platform.hh"
#include "engine/components.hh"
#include "engine/entity.hh"
#include "engine/registry.hh"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace YAML {

template <>
struct convert<zod::vec3> {
  static Node encode(const zod::vec3& v) {
    Node node;
    node.push_back(v.x);
    node.push_back(v.y);
    node.push_back(v.z);
    node.SetStyle(EmitterStyle::Flow);
    return node;
  }

  static bool decode(const Node& node, zod::vec3& v) {
    if (not node.IsSequence() or node.size() != 3) {
      return false;
    }

    v.x = node[0].as<zod::f32>();
    v.y = node[1].as<zod::f32>();
    v.z = node[2].as<zod::f32>();
    return true;
  }
};

template <>
struct convert<zod::vec4> {
  static Node encode(const zod::vec4& v) {
    Node node;
    node.push_back(v.x);
    node.push_back(v.y);
    node.push_back(v.z);
    node.push_back(v.w);
    node.SetStyle(EmitterStyle::Flow);
    return node;
  }

  static bool decode(const Node& node, zod::vec4& v) {
    if (not node.IsSequence() or node.size() != 4) {
      return false;
    }

    v.x = node[0].as<zod::f32>();
    v.y = node[1].as<zod::f32>();
    v.z = node[2].as<zod::f32>();
    v.w = node[3].as<zod::f32>();
    return true;
  }
};

template <>
struct convert<zod::mat4> {
  static Node encode(const zod::mat4& mat) {
    Node node;
    node.push_back(mat[0]);
    node.push_back(mat[1]);
    node.push_back(mat[2]);
    node.push_back(mat[3]);
    node.SetStyle(EmitterStyle::Flow);
    return node;
  }

  static bool decode(const Node& node, zod::mat4& mat) {
    if (not node.IsSequence() or node.size() != 4) {
      return false;
    }

    mat[0] = node[0].as<zod::vec4>();
    mat[1] = node[1].as<zod::vec4>();
    mat[2] = node[2].as<zod::vec4>();
    mat[3] = node[3].as<zod::vec4>();
    return true;
  }
};

} // namespace YAML

namespace zod {

auto operator<<(YAML::Emitter& out, const vec3& v) -> YAML::Emitter& {
  out << YAML::Flow;
  out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
  return out;
}

Scene::Scene()
    : m_mesh_batch(shared<GPUMeshBatch>()),
      m_camera_buffer(GPUBackend::get().create_storage_buffer()) {
  m_camera_buffer->upload_data(nullptr, sizeof(SceneData));
}

auto Scene::create() -> Entity {
  return create(fmt::format("empty.{}", next_id()));
}

auto Scene::create(const String& name) -> Entity {
  auto entity = Entity(m_registry.create(), this);
  entity.add_component<IdentifierComponent>(name);
  entity.add_component<UUIDComponent>(UUID());
  entity.add_component<TransformComponent>();
  return entity;
}

auto Scene::remove(Entity entity) -> void { entity.remove(); }

auto Scene::clear() -> void {
  m_camera = entt::null;
  m_env = entt::null;
  m_registry.clear();
}

auto Scene::update() -> void {
  if (m_camera == entt::null) {
    return;
  }
  auto entity = Entity(m_camera, this);
  auto camera = entity.get_component<CameraComponent>().camera;
  if (camera->is_dirty) {
    camera->is_dirty = false;
    auto storage = SceneData { camera->get_view(),
                               camera->get_projection(),
                               camera->get_inv_view(),
                               camera->get_inv_projection(),
                               vec4(camera->get_direction(), 0.0f),
                               vec4(camera->get_position(), 0.0f) };
    m_camera_buffer->upload_data(&storage, sizeof(SceneData));
  }
}

auto Scene::set_active_camera(Entity entity) -> void {
  m_camera = entity.m_inner;
}

auto Scene::active_camera() -> Entity { return Entity(m_camera, this); }

template <typename T>
static auto write_field(YAML::Emitter& out, const String& key, const T& value)
    -> void {
  out << YAML::Key << key << YAML::Value << value;
}

auto Scene::serialize(const fs::path& path) -> void {
  auto out = YAML::Emitter();
  out << YAML::BeginMap;
  out << YAML::Key << "Name" << YAML::Value << m_name;
  auto camera = Entity(m_camera, this).get_component<UUIDComponent>();
  auto camera_id = camera.id.to_string();
  out << YAML::Key << "CameraEntity" << YAML::Value << camera_id;
  out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
  m_registry.view<entt::entity>().each([&](auto entity_id) {
    auto entity = Entity(entity_id, this);
    if (not entity) {
      return;
    }

    out << YAML::BeginMap;
    write_field(out, "Entity",
                entity.get_component<UUIDComponent>().id.to_string());

    out << YAML::Key << "IdentifierComponent";
    out << YAML::BeginMap;
    const auto name =
        String(entity.get_component<IdentifierComponent>().identifier);
    write_field(out, "Identifier", name);
    out << YAML::EndMap;

    if (entity.has_component<TransformComponent>()) {
      out << YAML::Key << "TransformComponent";
      out << YAML::BeginMap;
      const auto& transform = entity.get_component<TransformComponent>();
      write_field(out, "Position", transform.position);
      write_field(out, "Rotation", transform.rotation);
      write_field(out, "Scale", transform.scale);
      out << YAML::EndMap;
    }

    if (entity.has_component<StaticMeshComponent>()) {
      out << YAML::Key << "StaticMeshComponent";
      out << YAML::BeginMap;
      const auto mesh = entity.get_component<StaticMeshComponent>().mesh;
      auto binary_path = fs::path("Meshes") / fmt::format("{}.zmesh", name);
      write_field(out, "Mesh", binary_path.string());
      auto ar = Archive();
      mesh->write(ar);
      ar.save(path / binary_path);
      out << YAML::EndMap;
    }

    if (entity.has_component<SkyboxComponent>()) {
      out << YAML::Key << "SkyboxComponent";
      out << YAML::BeginMap;
      const auto& env = entity.get_component<SkyboxComponent>().env;
      if (env.mode == LightingMode::SolidColor) {
        write_field(out, "LightingMode", "SolidColor");
        write_field(out, "Color", env.color.v3);
      } else if (env.mode == LightingMode::Texture) {
        write_field(out, "LightingMode", "Texture");
        write_field(out, "HDRI", env.hdr.s);
      } else {
        UNREACHABLE();
      }
      out << YAML::EndMap;
    }

    if (entity.has_component<CameraComponent>()) {
      out << YAML::Key << "CameraComponent";
      out << YAML::BeginMap;
      out << YAML::Key << "PerspectiveCamera" << YAML::Value << YAML::BeginMap;
      const auto camera = entity.get_component<CameraComponent>().camera;
      write_field(out, "Fov", camera->get_fov());
      auto [near, far] = camera->get_clipping();
      write_field(out, "ClipNear", near);
      write_field(out, "ClipFar", far);
      out << YAML::EndMap;
      out << YAML::EndMap;
    }

    out << YAML::EndMap;
  });
  out << YAML::EndSeq;
  out << YAML::EndMap;

  auto ar = Archive();
  ar.copy((u8*)out.c_str(), out.size());
  ar.save(path / "Scenes" / m_name, ".zscene");
}

auto Scene::deserialize(const fs::path& path) -> void {
  auto config = YAML::LoadFile(path);
  m_name = config["Name"].as<String>();
  auto camera_id = UUID(config["CameraEntity"].as<String>());
  const auto& entities = config["Entities"];
  ZASSERT(entities.IsSequence());
  for (const auto& node : entities) {
    ZASSERT(node["Entity"] and node["IdentifierComponent"]);
    auto entity = Entity(m_registry.create(), this);
    auto id = UUID(node["Entity"].as<String>());
    if (id == camera_id) {
      m_camera = entity.m_inner;
    }
    entity.add_component<UUIDComponent>(id);
    auto name = node["IdentifierComponent"]["Identifier"].as<String>();
    entity.add_component<IdentifierComponent>(name);

    if (node["TransformComponent"]) {
      auto tnode = node["TransformComponent"];
      auto& transform = entity.add_component<TransformComponent>();
      transform.position = tnode["Position"].as<vec3>();
      transform.rotation = tnode["Rotation"].as<vec3>();
      transform.scale = tnode["Scale"].as<vec3>();
    }

    if (node["CameraComponent"]) {
      const auto& cnode = node["CameraComponent"]["PerspectiveCamera"];
      auto& component = entity.add_component<CameraComponent>();
      auto& camera = component.camera;
      camera->set_fov(cnode["Fov"].as<f32>());
      camera->set_clipping(cnode["ClipNear"].as<f32>(),
                           cnode["ClipFar"].as<f32>());
      camera->update_matrix();
    }

    if (node["StaticMeshComponent"]) {
      auto mesh_path = node["StaticMeshComponent"]["Mesh"].as<String>();
      auto mesh = shared<Mesh>();
      auto root = path.parent_path().parent_path();
      mesh->read(root / mesh_path);
      entity.add_component<StaticMeshComponent>(mesh);
    }
  }

  const auto texconf =
      YAML::LoadFile(path.parent_path().parent_path() / "Textures.meta");
  const auto& textures = texconf["Textures"];
  auto offset = vec2();
  auto max_height = 0.0f;
  auto mega_texture = m_mesh_batch->mega_texture();
  mega_texture->bind();
  auto texture_info = Vector<TextureInfo>();
  for (const auto& node : textures) {
    const auto id = UUID(node["UUID"].as<String>());
    const auto texture_path = node["Path"].as<String>();
    auto mapping = memory_map(texture_path);
    auto size = vec2(*(usize*)mapping[0], *(usize*)mapping[sizeof(usize)]);
    auto span =
        Span(mapping[sizeof(usize) * 3], *(usize*)mapping[sizeof(usize) * 2]);
    if (offset.x + size.x > MEGA_TEXTURE_SIZE.x) {
      offset.x = 0.0f;
      offset.y += max_height;
      max_height = 0.0f;
    }
    mega_texture->blit(offset.x, offset.y, size.x, size.y, span.data());
    auto info = TextureInfo { offset, size };
    texture_info.push_back(info);
    AssetRegistry::reg(id, info);
    offset.x += size.x;
    max_height = fmaxf(max_height, size.y);
    memory_unmap(mapping);
  }
  mega_texture->generate_mipmap();

  m_mesh_batch->texture_info()->update_data(
      texture_info.data(), texture_info.size() * sizeof(TextureInfo));
}

template <class T>
auto Scene::on_component_added(Entity, T&) -> void {
  static_assert(false);
}

#define ON_COMPONENT_ADDED_DEFAULT_IMPL(Component)                             \
  template <>                                                                  \
  auto Scene::on_component_added<Component>(Entity, Component&)->void {}

ON_COMPONENT_ADDED_DEFAULT_IMPL(IdentifierComponent)
ON_COMPONENT_ADDED_DEFAULT_IMPL(UUIDComponent)
ON_COMPONENT_ADDED_DEFAULT_IMPL(CameraComponent)
ON_COMPONENT_ADDED_DEFAULT_IMPL(TransformComponent)

#undef ON_COMPONENT_ADDED_DEFAULT_IMPL

template <>
auto Scene::on_component_added<StaticMeshComponent>(Entity,
                                                    StaticMeshComponent&)
    -> void {
  m_mesh_batch->recompute_batch();
}

template <>
auto Scene::on_component_added<SkyboxComponent>(Entity entity, SkyboxComponent&)
    -> void {
  m_env = entity.m_inner;
}

}; // namespace zod
