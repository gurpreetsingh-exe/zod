#pragma once

#include "core/uuid.hh"
#include "engine/camera.hh"
#include "engine/environment.hh"
#include "engine/mesh.hh"

namespace YAML {
class Emitter;
class Node;
} // namespace YAML

namespace zod {

#define IMPL_SERDE()                                                           \
  void serialize(YAML::Emitter&);                                              \
  void deserialize(YAML::Node);

struct IdentifierComponent {
  char identifier[64] = {};
  IdentifierComponent() = default;
  IdentifierComponent(const String&);

  IMPL_SERDE()
};

struct UUIDComponent {
  UUID id;
  UUIDComponent() = delete;
  UUIDComponent(const UUID& uuid) : id(uuid) {}
  UUIDComponent(YAML::Node);

  IMPL_SERDE()
};

struct TransformComponent {
  vec3 position;
  vec3 rotation;
  vec3 scale = vec3(1.0f);

  TransformComponent() = default;
  TransformComponent(vec3 pos, vec3 rot, vec3 sc)
      : position(pos), rotation(rot), scale(sc) {}

  auto operator*() -> mat4;

  IMPL_SERDE()
};

struct CameraComponent {
  SharedPtr<PerspectiveCamera> camera =
      shared<PerspectiveCamera>(600.0f, 400.0f, 90.0f, 0.01f, 100.0f);
  CameraComponent() = default;
  CameraComponent(SharedPtr<PerspectiveCamera> _camera) : camera(_camera) {}

  IMPL_SERDE()
};

struct StaticMeshComponent {
  SharedPtr<Mesh> mesh = nullptr;
  StaticMeshComponent() = default;
  StaticMeshComponent(SharedPtr<Mesh> _mesh) : mesh(_mesh) {}

  IMPL_SERDE()
};

struct SkyboxComponent {
  Environment env = {};
  SkyboxComponent() = default;
  SkyboxComponent(Environment _env) : env(std::move(_env)) {}

  IMPL_SERDE()
};

enum class LightKind {
  Point,
  Sun,
};

struct LightComponent {
  LightKind kind = LightKind::Point;
  f32 a = 0.1f;
  f32 b = 2.0f;
  f32 intensity = 10.0f;

  IMPL_SERDE()
};

#undef IMPL_SERDE

}; // namespace zod
