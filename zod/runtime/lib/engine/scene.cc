#include "engine/scene.hh"
#include "./mesh_batch.hh"
#include "engine/components.hh"
#include "engine/entity.hh"

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
      m_uniform_buffer(
          GPUBackend::get().create_uniform_buffer(sizeof(SceneData))) {}

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

auto Scene::update() -> void {
  auto entity = Entity(m_camera, this);
  auto camera = entity.get_component<CameraComponent>().camera;
  if (camera->is_dirty) {
    camera->is_dirty = false;
    auto storage = SceneData { camera->get_view_projection(),
                               vec4(camera->get_direction(), 0.0f) };
    m_uniform_buffer->upload_data(&storage, sizeof(SceneData));
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

    out << YAML::EndMap;
  });
  out << YAML::EndSeq;
  out << YAML::EndMap;

  auto ar = Archive();
  ar.copy((u8*)out.c_str(), out.size());
  ar.save(path / "Scenes" / m_name, ".zscene");
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
