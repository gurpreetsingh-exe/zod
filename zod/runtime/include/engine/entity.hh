#pragma once

#include <entt/entt.hpp>

#include "engine/components.hh"
#include "engine/scene.hh"

namespace zod {

class Entity {
public:
  Entity() = default;
  Entity(entt::entity handle, Scene* scene) : m_inner(handle), m_scene(scene) {}
  Entity(const Entity&) = default;
  ~Entity() = default;

public:
  auto operator==(Entity other) const -> bool {
    return m_inner == other.m_inner;
  }
  explicit operator bool() {
    return m_inner != entt::null and m_scene != nullptr;
  }

public:
  template <typename T>
  auto has_component() -> bool {
    ZASSERT(*this, "default entity");
    return m_scene->m_registry.all_of<T>(m_inner);
  }

  template <typename T, typename... Args>
  auto add_component(Args&&... args) -> T& {
    ZASSERT(not has_component<T>(), "already has component");
    auto& component =
        m_scene->m_registry.emplace<T>(m_inner, std::forward<Args>(args)...);
    m_scene->on_component_added<T>(*this, component);
    return component;
  }

  template <typename T>
  auto get_component() -> T& {
    ZASSERT(has_component<T>(), "component not found");
    return m_scene->m_registry.get<T>(m_inner);
  }

  template <typename T>
  auto update_internal() -> void;

  auto remove() -> void { m_scene->m_registry.destroy(m_inner); }

private:
  entt::entity m_inner = entt::null;
  Scene* m_scene = nullptr;

  friend class Scene;
  friend struct std::hash<Entity>;
};

}; // namespace zod

namespace std {
template <>
struct hash<zod::Entity> {
  auto operator()(zod::Entity entity) const -> zod::usize {
    return hash<entt::entity>()(entity.m_inner);
  }
};
} // namespace std
