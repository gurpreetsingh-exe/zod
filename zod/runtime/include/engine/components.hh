#pragma once

#include "engine/camera.hh"
#include "engine/mesh.hh"

namespace zod {

struct IdentifierComponent {
  char identifier[64] = {};
  IdentifierComponent(const String& name) {
    auto sz = name.size() >= 64L ? 64L : name.size();
    std::memcpy(&identifier, name.c_str(), sz);
  }
};

struct TransformComponent {
  vec3 position;
  vec3 rotation;
  vec3 scale = vec3(1.0f);

  TransformComponent() = default;
  TransformComponent(vec3 pos, vec3 rot, vec3 sc)
      : position(pos), rotation(rot), scale(sc) {}

  auto operator*() -> mat4 {
    auto tmat = translate(mat4(1.0f), position);
    auto rmat = toMat4(quat(radians(rotation)));
    auto smat = glm::scale(mat4(1.0f), scale);
    return tmat * rmat * smat;
  }
};

struct CameraComponent {
  PerspectiveCamera camera { 600.0f, 400.0f, 90.0f, 0.01f, 100.0f };
  CameraComponent() = default;
  CameraComponent(PerspectiveCamera& _camera) : camera(_camera) {}
  ~CameraComponent() = default;
};

struct StaticMeshComponent {
  Mesh* mesh = nullptr;
  StaticMeshComponent() = default;
  StaticMeshComponent(Mesh* _mesh) : mesh(_mesh) {}
};

}; // namespace zod
