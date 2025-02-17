#pragma once

#include <entt/entt.hpp>

#include "scene.hh"

namespace zod {

class Entity {
public:
  Entity() = default;
  Entity(entt::entity handle, Scene* scene)
      : m_handle(handle), m_scene(scene) {}
  Entity(const Entity&) = default;
  ~Entity() = default;

public:
  auto operator==(Entity other) const -> bool {
    return m_handle == other.m_handle;
  }
  explicit operator bool() {
    return m_handle != entt::null and m_scene != nullptr;
  }

public:
  template <typename T>
  auto has_component() -> bool {
    ZASSERT(*this, "default entity");
    return m_scene->m_registry.all_of<T>(m_handle);
  }

  template <typename T, typename... Args>
  auto add_component(Args&&... args) -> T& {
    ZASSERT(not has_component<T>(), "already has component");
    return m_scene->m_registry.emplace<T>(m_handle,
                                          std::forward<Args>(args)...);
  }

  template <typename T>
  auto get_component() -> T& {
    ZASSERT(has_component<T>(), "component not found");
    return m_scene->m_registry.get<T>(m_handle);
  }

  auto remove() -> void { m_scene->m_registry.destroy(m_handle); }

private:
  entt::entity m_handle = entt::null;
  Scene* m_scene = nullptr;

  friend struct std::hash<Entity>;
};

}; // namespace zod

namespace std {
template <>
struct hash<zod::Entity> {
  auto operator()(zod::Entity entity) const -> zod::usize {
    return hash<entt::entity>()(entity.m_handle);
  }
};
} // namespace std
