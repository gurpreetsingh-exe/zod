#pragma once

#include "scene.hh"

namespace zod {

class Runtime {
public:
  Runtime() = default;
  static auto init() -> void;
  static auto destroy() -> void;

private:
  Unique<Scene> m_scene;
};

} // namespace zod
