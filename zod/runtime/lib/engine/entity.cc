#include "engine/entity.hh"
#include "./mesh_batch.hh"

namespace zod {

template <typename T>
auto Entity::update_internal() -> void {
  static_assert(false);
}

template <>
auto Entity::update_internal<CameraComponent>() -> void {}

template <>
auto Entity::update_internal<TransformComponent>() -> void {
  auto& component = get_component<TransformComponent>();
  m_scene->m_mesh_batch->update_matrix(*this, *component);
}

template <>
auto Entity::update_internal<SkyboxComponent>() -> void {
  auto& env = get_component<SkyboxComponent>().env;
  if (env.mode != LightingMode::Texture) {
    return;
  }
  m_scene->m_cubemap = m_scene->m_mesh_batch->load_env(env);
}

template <>
auto Entity::update_internal<StaticMeshComponent>() -> void {
  m_scene->m_mesh_batch->recompute_batch();
}

} // namespace zod
