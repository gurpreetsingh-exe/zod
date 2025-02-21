#pragma once

#include <entt/entt.hpp>

#include "application/event.hh"

namespace zod {

class Entity;
class Outliner;

class Scene {
public:
  Scene() = default;
  ~Scene() = default;

public:
  auto create() -> Entity;
  auto create(const std::string&) -> Entity;
  auto remove(Entity) -> void;
  auto update(Event&) -> void;
  auto operator->() const -> const entt::registry* { return &m_registry; }

private:
  auto next_id() -> usize { return m_disambiguator++; }

private:
  entt::registry m_registry;
  usize m_disambiguator = 0;
  friend class Entity;
  friend class Outliner;
  friend class ZCtxt;
};

}; // namespace zod
