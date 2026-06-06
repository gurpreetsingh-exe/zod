#include "engine/asset_database.hh"

#include <yaml-cpp/yaml.h>

namespace zod {

auto AssetDatabase::open(const fs::path& assets_directory) -> void {
  m_assets_directory = assets_directory;
  m_asset_records.clear();
  m_asset_record_map.clear();

  load_texture_meta();
}

auto AssetDatabase::register_asset(UUID id, AssetKind kind, fs::path path)
    -> void {
  auto record = AssetRecord {
    .id = id,
    .kind = kind,
    .path = resolve_path(path),
  };

  if (m_asset_record_map.contains(id)) {
    m_asset_records[m_asset_record_map[id]] = std::move(record);
    return;
  }

  m_asset_record_map[id] = m_asset_records.size();
  m_asset_records.push_back(std::move(record));
}

auto AssetDatabase::find(UUID id) const -> const AssetRecord* {
  if (not m_asset_record_map.contains(id)) {
    return nullptr;
  }

  return &m_asset_records.at(m_asset_record_map.at(id));
}

auto AssetDatabase::path(UUID id) const -> fs::path {
  auto* record = find(id);
  return record ? record->path : fs::path();
}

auto AssetDatabase::assets(AssetKind kind) const -> Vector<AssetRecord> {
  auto records = Vector<AssetRecord>();
  for (const auto& record : m_asset_records) {
    if (record.kind == kind) {
      records.push_back(record);
    }
  }
  return records;
}

auto AssetDatabase::assets_directory() const -> const fs::path& {
  return m_assets_directory;
}

auto AssetDatabase::load_texture_meta() -> void {
  auto meta_path = m_assets_directory / "Textures.meta";
  if (not fs::exists(meta_path)) {
    return;
  }

  auto config = YAML::LoadFile(meta_path);
  const auto& textures = config["Textures"];
  if (not textures or not textures.IsSequence()) {
    return;
  }

  for (const auto& node : textures) {
    if (not node["UUID"] or not node["Path"]) {
      continue;
    }

    register_asset(UUID(node["UUID"].as<String>()), AssetKind::Texture,
                   node["Path"].as<String>());
  }
}

auto AssetDatabase::resolve_path(const fs::path& path) const -> fs::path {
  if (path.is_absolute() or fs::exists(path)) {
    return path;
  }

  return m_assets_directory / path;
}

} // namespace zod
