#pragma once

#include <entt/entt.hpp>

#include "core/uuid.hh"
#include "gpu/backend.hh"

namespace zod {

class Entity;
class Outliner;
class Renderer;
class GPUMeshBatch;

struct SceneData {
  mat4 view;
  mat4 projection;
  mat4 inv_view;
  mat4 inv_projection;
  vec4 direction;
  vec4 position;
};

struct TextureInfo {
  vec2 offset;
  vec2 size;
};

class Scene {
public:
  Scene();
  ~Scene() = default;

public:
  auto create() -> Entity;
  auto create(const String&) -> Entity;
  auto remove(Entity) -> void;
  auto clear() -> void;
  auto update() -> void;
  auto active_camera() -> Entity;
  auto set_active_camera(Entity) -> void;
  auto operator->() const -> const entt::registry* { return &m_registry; }
  auto serialize(const fs::path&) -> void;
  auto deserialize(const fs::path&) -> void;
  auto on_component_added(Entity, auto&) -> void;
  auto name() const -> const String& { return m_name; }

private:
  auto next_id() -> usize { return m_disambiguator++; }

private:
  String m_name = "Default";
  entt::registry m_registry;
  usize m_disambiguator = 0;
  entt::entity m_camera = entt::null;
  entt::entity m_env = entt::null;
  std::unordered_map<UUID, entt::entity> m_entity_map = {};
  SharedPtr<GPUStorageBuffer> m_camera_buffer = nullptr;
  SharedPtr<GPUMeshBatch> m_mesh_batch = nullptr;
  SharedPtr<GPUTexture> m_cubemap = nullptr;
  friend class Entity;
  friend class Outliner;
  friend class Renderer;
  friend class ForwardRenderer;
  friend class DeferredRenderer;
};

}; // namespace zod
