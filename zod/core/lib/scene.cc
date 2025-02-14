#include "scene.hh"
#include "components.hh"
#include "entity.hh"

namespace zod {

auto Scene::create() -> Entity {
  return create(fmt::format("empty.{}", next_id()));
}

auto Scene::create(const std::string& name) -> Entity {
  auto entity = Entity(m_registry.create(), this);
  entity.add_component<IdentifierComponent>(name);
  entity.add_component<TransformComponent>();
  return entity;
}

auto Scene::remove(Entity entity) -> void { entity.remove(); }

auto Scene::update(Event& event) -> void {
  auto cameras = m_registry.view<CameraComponent>();
  for (auto& camera : cameras) {
    auto& component = m_registry.get<CameraComponent>(camera);
    component.camera.update(event);
  }
}

}; // namespace zod
