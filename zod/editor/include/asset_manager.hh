#pragma once

#include "core/uuid.hh"
#include "engine/registry.hh"
#include "loaders.hh"

namespace zod {

class AssetManager {
public:
  static auto load(const fs::path& path) -> UUID {
    // TODO: check if file has changed and reload it.
    auto code = std::error_code();
    const auto& canonical_path = fs::canonical(path, code);
    if (not fs::exists(canonical_path)) {
      return UUID::null();
    }
    if (not fs::is_regular_file(canonical_path)) {
      return UUID::null();
    }
    if (m_interned_paths.contains(canonical_path)) {
      return m_interned_paths[canonical_path];
    }
    auto uuid = UUID();
    m_interned_paths[canonical_path] = uuid;
    auto mesh = load_obj(path);
    AssetRegistry::reg<SharedPtr<Mesh>>(uuid, mesh);
    return uuid;
  }

  auto get(UUID uuid) const -> SharedPtr<Mesh> {
    return AssetRegistry::get<SharedPtr<Mesh>>(uuid);
  }

private:
  static inline std::unordered_map<fs::path, UUID> m_interned_paths;
};

} // namespace zod
