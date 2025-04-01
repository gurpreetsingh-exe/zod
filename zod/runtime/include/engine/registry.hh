#pragma once

#include "core/uuid.hh"
#include "engine/mesh.hh"
#include "engine/scene.hh"

namespace zod {

template <class... Ts>
using Tuple = std::tuple<Ts...>;

template <class T>
using UUIDMap = std::unordered_map<UUID, T>;

class AssetRegistry {
public:
  template <class T>
  static constexpr auto reg(UUID id, const T& resource) -> void {
    auto& map = std::get<UUIDMap<T>>(s_registry);
    map[id] = resource;
  }

  template <class T>
  static constexpr auto get(UUID id) -> const T& {
    return std::get<UUIDMap<T>>(s_registry)[id];
  }

private:
  static inline Tuple<UUIDMap<SharedPtr<Mesh>>, UUIDMap<SharedPtr<Scene>>,
                      UUIDMap<TextureInfo>>
      s_registry;
};

} // namespace zod
