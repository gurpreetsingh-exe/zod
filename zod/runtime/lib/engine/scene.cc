#include "engine/scene.hh"
#include "./mesh_batch.hh"
#include "core/platform/platform.hh"
#include "engine/components.hh"
#include "engine/entity.hh"
#include "engine/project.hh"
#include "engine/registry.hh"

#include <yaml-cpp/yaml.h>

namespace zod {

Scene::Scene()
    : m_mesh_batch(shared<GPUMeshBatch>()),
      m_camera_buffer(GPUBackend::get().create_buffer(
          { "scene.camera", GPUBufferUsage::Storage, sizeof(SceneData) })) {
  auto entity = create("SceneCamera");
  entity.add_component<CameraComponent>();
  m_camera = entity.m_inner;
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
    m_camera_buffer->write(&storage, sizeof(SceneData));
  }
}

auto Scene::set_active_camera(Entity entity) -> void {
  m_camera = entity.m_inner;
}

auto Scene::active_camera() -> Entity { return Entity(m_camera, this); }

auto Scene::entities() -> Vector<Entity> {
  auto entities = Vector<Entity>();
  m_registry.view<IdentifierComponent>().each(
      [&](auto entity, const IdentifierComponent&) {
        entities.emplace_back(entity, this);
      });
  return entities;
}

auto Scene::mega_texture() const -> SharedPtr<GPUTexture> {
  return m_mesh_batch->mega_texture();
}

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
#define SERIALIZE_COMPONENT(Component)                                         \
  entity.has_component_then<Component>([&out](auto& component) {               \
    out << YAML::Key << #Component;                                            \
    component.serialize(out);                                                  \
  })

    auto name = String(entity.get_component<IdentifierComponent>().identifier);

    out << YAML::Key << "UUIDComponent";
    entity.get_component<UUIDComponent>().serialize(out);

    out << YAML::Key << "IdentifierComponent";
    entity.get_component<IdentifierComponent>().serialize(out);

    SERIALIZE_COMPONENT(TransformComponent);

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

    SERIALIZE_COMPONENT(SkyboxComponent);
    SERIALIZE_COMPONENT(CameraComponent);
    SERIALIZE_COMPONENT(LightComponent);

#undef SERIALIZE_COMPONENT

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
    ZASSERT(node["UUIDComponent"] and node["IdentifierComponent"]);
    auto entity = Entity(m_registry.create(), this);
    entity.add_component<IdentifierComponent>().deserialize(
        node["IdentifierComponent"]);
    auto id = entity.add_component<UUIDComponent>(node["UUIDComponent"]).id;
    if (id == camera_id) {
      auto camera_entity = Entity(m_camera, this);
      if (camera_entity) {
        remove(camera_entity);
      }
      m_camera = entity.m_inner;
    }

#define DESERIALIZE_COMPONENT(Component)                                       \
  if (node[#Component]) {                                                      \
    entity.add_component<Component>().deserialize(node[#Component]);           \
    entity.update_internal<Component>();                                       \
  }

    DESERIALIZE_COMPONENT(TransformComponent);

    if (node["StaticMeshComponent"]) {
      auto mesh_path = node["StaticMeshComponent"]["Mesh"].as<String>();
      auto mesh = shared<Mesh>();
      auto root = path.parent_path().parent_path();
      mesh->read(root / mesh_path);
      entity.add_component<StaticMeshComponent>(mesh);
    }

    DESERIALIZE_COMPONENT(CameraComponent);
    DESERIALIZE_COMPONENT(SkyboxComponent);
    DESERIALIZE_COMPONENT(LightComponent);

#undef DESERIALIZE_COMPONENT
  }

  const auto textures = g_project->asset_database().mega_texture_assets();
  auto offset = vec2();
  auto max_height = 0.0f;
  auto mega_texture = m_mesh_batch->mega_texture();
  mega_texture->bind();
  auto texture_info = Vector<TextureInfo>();
  for (const auto& texture : textures) {
    auto mapping = memory_map(texture.path);
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
    AssetRegistry::reg(texture.id, info);
    offset.x += size.x;
    max_height = fmaxf(max_height, size.y);
    memory_unmap(mapping);
  }
  mega_texture->generate_mipmap();

  m_mesh_batch->texture_info()->write(
      texture_info.data(), texture_info.size() * sizeof(TextureInfo));
  m_mesh_batch->recompute_batch();
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

template <>
auto Scene::on_component_added<LightComponent>(Entity, LightComponent&)
    -> void {
  m_mesh_batch->recompute_batch();
  m_stats.number_lights += 1;
}

}; // namespace zod
