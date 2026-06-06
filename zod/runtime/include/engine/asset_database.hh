#pragma once

#include "core/uuid.hh"

namespace zod {

enum class AssetKind {
  Texture,
};

struct AssetRecord {
  UUID id;
  AssetKind kind;
  fs::path path;
  fs::path source;
  fs::path meta;
};

class AssetDatabase {
public:
  auto open(const fs::path& assets_directory) -> void;
  auto register_asset(UUID id, AssetKind kind, fs::path path,
                      fs::path source = {}, fs::path meta = {}) -> void;
  auto find(UUID id) const -> const AssetRecord*;
  auto path(UUID id) const -> fs::path;
  auto assets(AssetKind kind) const -> Vector<AssetRecord>;
  auto set_mega_texture_assets(Vector<UUID> assets) -> void;
  auto mega_texture_assets() const -> Vector<AssetRecord>;
  auto assets_directory() const -> const fs::path&;

private:
  auto load_texture_metas() -> void;
  auto load_legacy_texture_meta() -> void;
  auto load_mega_texture_meta() -> void;
  auto resolve_path(const fs::path& path) const -> fs::path;

private:
  fs::path m_assets_directory = {};
  Vector<AssetRecord> m_asset_records = {};
  std::unordered_map<UUID, usize> m_asset_record_map = {};
  Vector<UUID> m_mega_texture_assets = {};
};

} // namespace zod
