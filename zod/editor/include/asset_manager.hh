#pragma once

#include "core/io/obj.hh"
#include "core/uuid.hh"

namespace zod {

class AssetManager {
public:
  auto load(const fs::path& path) -> uuid {
    auto code = std::error_code();
    const auto& canonical_path = fs::canonical(path, code);
    if (not fs::exists(canonical_path)) {
      return uuid0();
    }
    if (not fs::is_regular_file(canonical_path)) {
      return uuid0();
    }
    if (m_interned_paths.contains(canonical_path)) {
      return m_interned_paths[canonical_path];
    }
    auto uuid = m_generator.getUUID();
    m_interned_paths[canonical_path] = uuid;
    ZASSERT(not m_meshes.contains(uuid));
    auto* mesh = load_obj(path);
    m_meshes[uuid] = mesh;
    return uuid;
  }

  auto get(uuid uuid) -> Mesh* { return m_meshes[uuid]; }

private:
  std::unordered_map<uuid, Mesh*> m_meshes;
  std::unordered_map<fs::path, uuid> m_interned_paths;
  UUIDv4::UUIDGenerator<std::mt19937_64> m_generator;
};

// template <typename T1, typename T2>
// class AssetManager;

// template <class... Ts, class... Fns>
// class AssetManager<std::tuple<Ts...>, std::tuple<Fns...>> {
//   using map_type = std::unordered_map<uuid, Ts...>;
//   using pair_type = std::pair<map_type, Fns...>;

// public:
//   AssetManager() = default;

// public:
//   auto load(const fs::path& path) -> uuid {
//     const auto& canonical_path = fs::canonical(path);
//     if (m_interned_paths.contains(canonical_path)) {
//       return m_interned_paths[canonical_path];
//     }
//     auto uuid = m_generator.getUUID();
//     m_interned_paths[canonical_path] = uuid;
//     auto& [map, fn] = std::get<pair_type>(m_resources);
//     ZASSERT(not map.contains(uuid));
//     const auto& asset = fn(path);
//     // static_assert(std::is_same<decltype(asset), >::value);
//     static_assert(std::is_same<decltype(asset), typename
//     map_type::value_type>::value); map[uuid] = asset; return uuid;
//   }

// private:
//   std::tuple<pair_type> m_resources;
//   std::unordered_map<fs::path, uuid> m_interned_paths;
//   UUIDv4::UUIDGenerator<std::mt19937_64> m_generator;
// };

} // namespace zod
