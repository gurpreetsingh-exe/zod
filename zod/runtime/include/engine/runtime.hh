#pragma once

#include "scene.hh"

namespace zod {

class Runtime {
public:
  Runtime();
  static auto init() -> void;
  static auto destroy() -> void;
  static auto get() -> Runtime&;
  auto scene() -> Scene&;

private:
  UUID m_scene;
};

} // namespace zod
