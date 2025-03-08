#pragma once

namespace zod {

struct Project {
  String name;
  fs::path directory;
  fs::path assets = "Assets";

  auto init() -> void;
  auto exists() const -> bool;
  auto save() const -> void;
  auto assets_directory() const -> fs::path;
};

} // namespace zod
