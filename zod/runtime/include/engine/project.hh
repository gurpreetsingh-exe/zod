#pragma once

namespace zod {

class Scene;

class SceneManager {
public:
  auto load_from_file(const fs::path&) -> void;
  auto active_scene() -> SharedPtr<Scene>;

private:
  Vector<SharedPtr<Scene>> m_scenes = {};
  usize m_active_scene = 0;
};

class Project {
public:
  String name;
  fs::path directory;
  fs::path assets = "Assets";

public:
  Project(String name, fs::path directory, fs::path assets = "Assets")
      : name(std::move(name)),
        directory(directory.empty() ? fs::current_path()
                                    : std::move(directory)),
        assets(std::move(assets)), m_scene_manager() {}

  auto init() -> void;
  auto exists() const -> bool;
  auto save() -> void;
  static auto load(const fs::path&) -> void;
  auto assets_directory() const -> fs::path;
  auto scene_manager() -> SceneManager& { return m_scene_manager; }
  auto active_scene() -> SharedPtr<Scene> {
    return m_scene_manager.active_scene();
  }

private:
  SceneManager m_scene_manager;
};

extern Project* g_project;

} // namespace zod
