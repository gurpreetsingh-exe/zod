#pragma once

#include "scene.hh"

namespace zod {

class Runtime {
public:
  Runtime() : m_scene(unique<Scene>()) {}
  static auto init() -> void;
  static auto destroy() -> void;
  static auto get() -> Runtime&;
  auto scene() -> Scene&;

private:
  Unique<Scene> m_scene;
};

} // namespace zod
