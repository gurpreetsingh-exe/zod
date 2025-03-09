#pragma once

namespace zod {

struct Project {
  String name;
  fs::path directory;
  fs::path assets = "Assets";

  auto init() -> void;
  auto exists() const -> bool;
  auto save() const -> void;
  static auto load(const fs::path&) -> Project*;
  auto assets_directory() const -> fs::path;
};

} // namespace zod
