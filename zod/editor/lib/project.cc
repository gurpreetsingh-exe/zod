#include "project.hh"
#include "core/serialize/archive.hh"
#include "engine/runtime.hh"
#include <yaml-cpp/yaml.h>

namespace zod {

auto Project::init() -> void {
  auto code = std::error_code();
  auto assets_dir = assets_directory();
  fs::create_directories(assets_dir / "Scenes", code);
  fs::create_directories(assets_dir / "Meshes", code);
}

auto Project::exists() const -> bool {
  auto config = directory / fmt::format("{}.zproj", name);
  return fs::exists(config);
}

auto Project::save() const -> void {
  auto& scene = Runtime::get().scene();
  scene.serialize(assets_directory());
  auto ar = Archive();
  auto out = YAML::Emitter();
  out << YAML::BeginMap;
  out << YAML::Key << "Name" << YAML::Value << name;
  out << YAML::Key << "AssetsDirectory" << YAML::Value << assets;
  out << YAML::Key << "StartScene" << YAML::Value
      << fs::path("Scenes") / fmt::format("{}.zscene", scene.name());
  out << YAML::EndMap;
  ar.copy((u8*)out.c_str(), out.size());
  ar.save(directory / name, ".zproj");
}

auto Project::load(const fs::path& path) -> Project* {
  auto config = YAML::LoadFile(path);
  const auto name = config["Name"].as<String>();
  const auto directory = path.parent_path();
  const auto assets = config["AssetsDirectory"].as<String>();
  const auto start_scene = config["StartScene"].as<String>();
  auto* project = new Project(name, directory, assets);
  auto& scene = Runtime::get().scene();
  scene.clear();
  scene.deserialize(project->assets_directory() / start_scene);
  return project;
}

auto Project::assets_directory() const -> fs::path {
  return directory / assets;
}

} // namespace zod
