#include "engine/asset_database.hh"

#include <yaml-cpp/yaml.h>

namespace zod {

auto AssetDatabase::open(const fs::path& assets_directory) -> void {
  m_assets_directory = assets_directory;
  m_asset_records.clear();
  m_asset_record_map.clear();
  m_materials.clear();
  m_mega_texture_indices.clear();
  m_mega_texture_assets.clear();

  load_texture_metas();
  load_material_metas();
  load_legacy_texture_meta();
  load_mega_texture_meta();
}

auto AssetDatabase::register_asset(UUID id, AssetKind kind, fs::path path,
                                   fs::path source, fs::path meta) -> void {
  auto record = AssetRecord {
    .id = id,
    .kind = kind,
    .path = resolve_path(path),
    .source = std::move(source),
    .meta = resolve_path(meta),
  };

  if (m_asset_record_map.contains(id)) {
    m_asset_records[m_asset_record_map[id]] = std::move(record);
    return;
  }

  m_asset_record_map[id] = m_asset_records.size();
  m_asset_records.push_back(std::move(record));
}

auto AssetDatabase::register_material(UUID id, const Material& material)
    -> void {
  m_materials[id] = material;
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

auto AssetDatabase::material(UUID id) const -> Material {
  if (not m_materials.contains(id)) {
    return {};
  }

  return m_materials.at(id);
}

auto AssetDatabase::texture_index(UUID id) const -> usize {
  if (id == UUID::null() or not m_mega_texture_indices.contains(id)) {
    return -1ZU;
  }

  return m_mega_texture_indices.at(id);
}

auto AssetDatabase::set_mega_texture_assets(Vector<UUID> assets) -> void {
  m_mega_texture_assets = std::move(assets);
  m_mega_texture_indices.clear();
  for (auto i = 0ZU; i < m_mega_texture_assets.size(); ++i) {
    m_mega_texture_indices[m_mega_texture_assets[i]] = i;
  }
}

auto AssetDatabase::mega_texture_assets() const -> Vector<AssetRecord> {
  auto records = Vector<AssetRecord>();
  for (const auto& id : m_mega_texture_assets) {
    auto* record = find(id);
    if (record) {
      records.push_back(*record);
    }
  }

  return records.empty() ? assets(AssetKind::Texture) : records;
}

auto AssetDatabase::assets_directory() const -> const fs::path& {
  return m_assets_directory;
}

auto AssetDatabase::load_texture_metas() -> void {
  auto textures_dir = m_assets_directory / "Textures";
  if (not fs::exists(textures_dir)) {
    return;
  }

  for (const auto& entry : fs::recursive_directory_iterator(textures_dir)) {
    auto code = std::error_code();
    if (not entry.is_regular_file(code) or
        entry.path().extension() != ".meta") {
      continue;
    }

    auto config = YAML::LoadFile(entry.path());
    if (not config["UUID"] or not config["Artifact"]) {
      continue;
    }

    auto source = fs::path();
    if (config["Source"]) {
      source = config["Source"].as<String>();
    }
    register_asset(UUID(config["UUID"].as<String>()), AssetKind::Texture,
                   config["Artifact"].as<String>(), source, entry.path());
  }
}

auto AssetDatabase::load_material_metas() -> void {
  auto materials_dir = m_assets_directory / "Materials";
  if (not fs::exists(materials_dir)) {
    return;
  }

  for (const auto& entry : fs::recursive_directory_iterator(materials_dir)) {
    auto code = std::error_code();
    if (not entry.is_regular_file(code) or
        entry.path().extension() != ".meta") {
      continue;
    }

    auto config = YAML::LoadFile(entry.path());
    if (not config["UUID"]) {
      continue;
    }

    auto id = UUID(config["UUID"].as<String>());
    auto source = fs::path();
    if (config["Source"]) {
      source = config["Source"].as<String>();
    }

    auto texture = [](const YAML::Node& node, const char* name) {
      return node[name] ? UUID(node[name].as<String>()) : UUID::null();
    };

    auto material = Material {
      .color_texture = texture(config, "BaseColorTexture"),
      .normal_texture = texture(config, "NormalTexture"),
      .roughness_texture = texture(config, "RoughnessTexture"),
    };
    register_asset(id, AssetKind::Material, entry.path(), source, entry.path());
    register_material(id, material);
  }
}

auto AssetDatabase::load_legacy_texture_meta() -> void {
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

auto AssetDatabase::load_mega_texture_meta() -> void {
  auto meta_path = m_assets_directory / "MegaTexture.meta";
  if (not fs::exists(meta_path)) {
    return;
  }

  auto config = YAML::LoadFile(meta_path);
  const auto& textures = config["Textures"];
  if (not textures or not textures.IsSequence()) {
    return;
  }

  for (const auto& node : textures) {
    if (node["UUID"]) {
      auto id = UUID(node["UUID"].as<String>());
      m_mega_texture_indices[id] = m_mega_texture_assets.size();
      m_mega_texture_assets.push_back(id);
    }
  }
}

auto AssetDatabase::resolve_path(const fs::path& path) const -> fs::path {
  if (path.empty()) {
    return {};
  }

  if (path.is_absolute() or fs::exists(path)) {
    return path;
  }

  return m_assets_directory / path;
}

} // namespace zod
