#include "engine/project.hh"
#include "core/serialize/archive.hh"
#include "engine/scene.hh"
#include <yaml-cpp/yaml.h>

namespace zod {

auto SceneManager::load_from_file(const fs::path& path) -> void {
  auto scene = create_scene();
  scene->deserialize(path);
}

auto SceneManager::active_scene() -> SharedPtr<Scene> {
  return m_scenes[m_active_scene];
}

auto SceneManager::create_scene() -> SharedPtr<Scene> {
  auto scene = shared<Scene>();
  m_scenes.push_back(scene);
  return scene;
}

Project* g_project = nullptr;

auto Project::init() -> void {
  auto code = std::error_code();
  auto assets_dir = assets_directory();
  fs::create_directories(assets_dir / "Scenes", code);
  fs::create_directories(assets_dir / "Meshes", code);
  fs::create_directories(assets_dir / "Textures", code);
  fs::create_directories(assets_dir / "Materials", code);
  m_scene_manager.create_scene();
  save();
}

auto Project::exists() const -> bool {
  auto config = directory / fmt::format("{}.zproj", name);
  return fs::exists(config);
}

auto Project::save() -> void {
  auto scene = m_scene_manager.active_scene();
  scene->serialize(assets_directory());
  auto ar = Archive();
  auto out = YAML::Emitter();
  out << YAML::BeginMap;
  out << YAML::Key << "Name" << YAML::Value << name;
  out << YAML::Key << "AssetsDirectory" << YAML::Value << assets;
  out << YAML::Key << "StartScene" << YAML::Value
      << fs::path("Scenes") / fmt::format("{}.zscene", scene->name());
  out << YAML::EndMap;
  ar.copy((u8*)out.c_str(), out.size());
  ar.save(directory / name, ".zproj");
}

auto Project::load(const fs::path& path) -> void {
  auto config = YAML::LoadFile(path);
  const auto name = config["Name"].as<String>();
  const auto directory = path.parent_path();
  const auto assets = config["AssetsDirectory"].as<String>();
  const auto start_scene = config["StartScene"].as<String>();
  g_project = new Project(name, directory, assets);
  g_project->m_scene_manager.load_from_file(g_project->assets_directory() /
                                            start_scene);
}

auto Project::assets_directory() const -> fs::path {
  return directory / assets;
}

} // namespace zod
